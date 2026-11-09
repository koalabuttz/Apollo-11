#!/usr/bin/env python3
"""evidence.py download fallback: the API route reaches the asset through a
redirect without forwarding the token (#1256), and the SHA256 check holds.

Two local servers stand in for GitHub: "cdn" answers 504 on the download URL
and serves the asset bytes only to requests with no Authorization header;
"api" needs the token, lists the release and 302s the asset to "cdn".
Two more assets take the CDN route only: "ok" is served at once (a SHA256
mismatch there must exit without a retry or the API), and "missing" is a
404 (raised at once, never retried or sent to the API)."""
import collections
import hashlib
import http.server
import json
import os
import pathlib
import sys
import tempfile
import threading
import urllib.request

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import evidence  # noqa: E402

HITS = collections.Counter()

DATA = b'evidence bytes\n'
TOKEN = 'test-token'


def serve(handler):
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server, f'http://127.0.0.1:{server.server_port}'


class Cdn(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args):
        pass

    def do_GET(self):
        HITS['cdn ' + self.path] += 1
        if self.path.endswith('/ok-x.log'):
            self.send_response(200)
            self.end_headers()
            self.wfile.write(DATA)
        elif self.path.endswith('/missing-x.log'):
            self.send_error(404)
        elif self.path.startswith('/releases/download/'):
            self.send_error(504)
        elif self.headers.get('Authorization'):
            self.send_error(400, 'token forwarded to the CDN')
        else:
            self.send_response(200)
            self.end_headers()
            self.wfile.write(DATA)


def main():
    cdn, cdn_url = serve(Cdn)

    class Api(http.server.BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass

        def do_GET(self):
            HITS['api'] += 1
            if self.headers.get('Authorization') != f'Bearer {TOKEN}':
                self.send_error(401)
            elif self.path == '/repos/o/r/releases/tags/evidence-m':
                body = json.dumps({'assets': [{'name': 'abc-x.log', 'url': f'{api_url}/assets/1'},
                                              {'name': 'def-x.log', 'url': f'{api_url}/assets/2'}]}).encode()
                self.send_response(200)
                self.end_headers()
                self.wfile.write(body)
            elif self.path in ('/assets/1', '/assets/2'):
                self.send_response(302)
                name = 'abc-x.log' if self.path.endswith('1') else 'def-x.log'
                self.send_header('Location', f'{cdn_url}/signed/{name}')
                self.end_headers()
            else:
                self.send_error(404)

    api, api_url = serve(Api)
    os.environ['GITHUB_TOKEN'] = TOKEN
    evidence.time.sleep = lambda seconds: None
    # Route github.com / api.github.com to the local servers.
    opener = urllib.request.urlopen

    def urlopen(request, timeout=0):
        url = request if isinstance(request, str) else request.full_url
        url = url.replace('https://github.com/o/r', cdn_url).replace('https://api.github.com', api_url)
        if isinstance(request, str):
            return opener(url, timeout=timeout)
        request.full_url = url
        return opener(request, timeout=timeout)

    evidence.urllib.request.urlopen = urlopen
    failures = []
    url = 'https://github.com/o/r/releases/download/evidence-m/'
    good = {'sha256': hashlib.sha256(DATA).hexdigest()}
    bad = {'sha256': '0' * 64}

    def case(name, run):
        evidence.USE_API, evidence.RELEASES = False, {}
        HITS.clear()
        try:
            run()
        except BaseException as error:  # a failing case reports, never tracebacks
            failures.append(f'{name}: {type(error).__name__}: {error}')

    def expect_mismatch(target, record, file):
        try:
            evidence.download(url + file, target, record)
        except SystemExit as error:
            if 'SHA256 mismatch' not in str(error):
                raise
            return
        failures.append(f'{file}: SHA256 mismatch was accepted')

    with tempfile.TemporaryDirectory() as tmp:
        tmp = pathlib.Path(tmp)

        def fallback():
            evidence.download(url + 'abc-x.log', tmp / 'a.log', good)
            if (tmp / 'a.log').read_bytes() != DATA:
                failures.append('API fallback returned the wrong bytes')
            # After one fallback, later downloads in the run go straight to the API.
            evidence.download(url + 'def-x.log', tmp / 'e.log', good)
            if HITS['cdn /releases/download/evidence-m/def-x.log'] or HITS['cdn /signed/def-x.log'] != 1:
                failures.append(f'later download did not go straight to the API: {dict(HITS)}')

        def api_mismatch():
            expect_mismatch(tmp / 'b.log', bad, 'abc-x.log')
            if HITS['cdn /signed/abc-x.log'] != 1:
                failures.append(f'API-route mismatch was retried: {dict(HITS)}')

        def cdn_mismatch():
            expect_mismatch(tmp / 'c.log', bad, 'ok-x.log')
            if HITS['cdn /releases/download/evidence-m/ok-x.log'] != 1 or HITS['api']:
                failures.append(f'CDN mismatch was retried: {dict(HITS)}')
            if any(tmp.glob('.download-*')) or (tmp / 'c.log').exists():
                failures.append('CDN mismatch left a file behind')

        def not_found():
            try:
                evidence.download(url + 'missing-x.log', tmp / 'd.log', good)
                failures.append('404 was not raised')
            except evidence.urllib.error.HTTPError as error:
                if error.code != 404:
                    raise
            if HITS['cdn /releases/download/evidence-m/missing-x.log'] != 1 or HITS['api'] or evidence.USE_API:
                failures.append(f'404 was retried or sent to the API: {dict(HITS)}')

        case('fallback', fallback)
        case('API-route mismatch', api_mismatch)
        case('CDN-route mismatch', cdn_mismatch)
        case('404', not_found)
    cdn.shutdown()
    api.shutdown()
    for failure in failures:
        print('FAIL:', failure, file=sys.stderr)
    if not failures:
        print('PASS: evidence API fallback follows the redirect without the token; '
              'SHA256 checked on both routes and not retried; 404 not retried')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
