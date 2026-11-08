#!/usr/bin/env python3
"""evidence.py download fallback: the API route reaches the asset through a
redirect without forwarding the token (#1256), and the SHA256 check holds.

Two local servers stand in for GitHub: "cdn" answers 504 on the download URL
and serves the asset bytes only to requests with no Authorization header;
"api" needs the token, lists the release and 302s the asset to "cdn"."""
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
        if self.path.startswith('/releases/download/'):
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
            if self.headers.get('Authorization') != f'Bearer {TOKEN}':
                self.send_error(401)
            elif self.path == '/repos/o/r/releases/tags/evidence-m':
                body = json.dumps({'assets': [{'name': 'abc-x.log', 'url': f'{api_url}/assets/1'}]}).encode()
                self.send_response(200)
                self.end_headers()
                self.wfile.write(body)
            elif self.path == '/assets/1':
                self.send_response(302)
                self.send_header('Location', f'{cdn_url}/signed/abc-x.log')
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
    with tempfile.TemporaryDirectory() as tmp:
        target = pathlib.Path(tmp) / 'x.log'
        record = {'sha256': hashlib.sha256(DATA).hexdigest()}
        evidence.download('https://github.com/o/r/releases/download/evidence-m/abc-x.log', target, record)
        if target.read_bytes() != DATA:
            failures.append('API fallback returned the wrong bytes')
        evidence.USE_API, evidence.RELEASES = False, {}
        try:
            evidence.download('https://github.com/o/r/releases/download/evidence-m/abc-x.log',
                              pathlib.Path(tmp) / 'y.log', {'sha256': '0' * 64})
            failures.append('SHA256 mismatch was accepted')
        except SystemExit as error:
            if 'SHA256 mismatch' not in str(error):
                failures.append(f'unexpected exit: {error}')
    cdn.shutdown()
    api.shutdown()
    for failure in failures:
        print('FAIL:', failure, file=sys.stderr)
    if not failures:
        print('PASS: evidence API fallback follows the redirect without the token; SHA256 checked')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
