#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The vice manifest: which vice-* targets the nightly ran, on which PRGs, with what result.

Every full port-ci run (nightly, dispatch) writes one piece per VICE job from
tools/ci_run.py's summary and logs, merges them in its vice-manifest job and
uploads the result as the workflow artifact `vice-manifest`
(vice-manifest.json). tools/cert_precheck.py checks each certificate's
vice_targets against the newest complete one, so C64 evidence follows the tip
instead of the PRG hash a run record named when it was captured.

  python3 tools/vice_manifest.py piece --name c64 --out build/vice-manifest-c64.json [--certificates]
  python3 tools/vice_manifest.py merge build/vice-pieces/*.json --job c64=success ... --out build/vice-manifest.json
  python3 tools/vice_manifest.py fetch          # newest complete nightly manifest into build/vice-manifest/

Manifest (schema 1): commit, ref, event, run_id, run_attempt, date (UTC),
complete (every VICE job finished, none cancelled or skipped), jobs, targets
{name: {status pass|fail, checks, failures, prg_sha256 {prg: sha256}, commit}}
and certificates {AGC path: {certificate key: vice_targets}} at the commit.
checks sums the run's PASS lines: tools/vice_run.py's mailbox count, else a
legacy script's first `N checks/cases/states/...` figure, else 1 per line.
"""
import argparse
import datetime
import hashlib
import io
import json
import os
import pathlib
import re
import subprocess
import sys
import urllib.error
import urllib.request
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCHEMA = 1
NAME = 'vice-manifest'
BRANCH = 'port/c64-native-c'
CACHE = ROOT / 'build' / NAME
CERTIFICATES = ('pending_complete_certificate', 'complete_native_coverage')
COUNT = re.compile(r"\b(\d[\d,]*) (?:[\w-]+ ){0,4}(?:checks|cases|states|scenarios|vectors|rows|words|sequences|bindings)\b|"
                   r"'(?:checks|cases|states)': (\d+)")
LOGGED_PRG = re.compile(r'PRG sha256: ([0-9a-f]{64}) ([\w.-]+\.prg)\b', re.I)
PRG = re.compile(r'(?<![\w.-])([\w.-]+\.prg)\b')


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run_result(text, ok, script=''):
    """(checks, failures, prg_sha256) from one target's log (and a legacy script's PRG names)."""
    checks = 0
    for line in text.splitlines():
        if re.match(r'\s*PASS\b', line):
            found = COUNT.search(line)
            checks += int((found[1] or found[2]).replace(',', '')) if found else 1
    failures = sum(int(n) for n in re.findall(r'\b(\d+) failures\b', text))
    prgs = {name: digest for digest, name in LOGGED_PRG.findall(text)}
    for name in PRG.findall(text + script):
        if name not in prgs and (ROOT / 'build' / name).is_file():
            prgs[name] = sha256(ROOT / 'build' / name)
    return checks if ok else 0, failures if ok else max(failures, 1), dict(sorted(prgs.items()))


def normalized(targets):
    """vice_targets as {target: certified checks}; a list certifies a pass (1 check)."""
    if isinstance(targets, list):
        targets = dict.fromkeys(targets, 1)
    if not isinstance(targets, dict) or not targets or not all(
            isinstance(k, str) and k.startswith('vice-') and isinstance(v, int) and not isinstance(v, bool)
            and v > 0 for k, v in targets.items()):
        return None
    return dict(sorted(targets.items()))


def certificates():
    found = {}
    for entry in json.loads((ROOT / 'docs/port-coverage.json').read_text()).get('files', []):
        for key in CERTIFICATES:
            record = ((entry.get(key) or {}).get('evidence') or {}).get('c64_execution_results') or {}
            if isinstance(record, dict) and normalized(record.get('vice_targets')):
                found.setdefault(entry['path'], {})[key] = normalized(record['vice_targets'])
    return found


def cmd_piece(args):
    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    try:
        results = json.loads(args.summary.read_text())['results']
    except (OSError, ValueError, KeyError):
        results = []
    targets = {}
    for row in sorted(results, key=lambda r: r['target']):
        if not row['target'].startswith('vice-'):
            continue
        log = args.log_dir / (re.sub(r'[^A-Za-z0-9._-]', '_', row['target']) + '.log')
        text = log.read_text(errors='replace') if log.is_file() else ''
        script = ROOT / 'tools' / (row['target'][len('vice-legacy-'):].replace('-', '_') + '_vice_test.py')
        legacy = script.read_text(errors='replace') if row['target'].startswith('vice-legacy-') and \
            script.is_file() else ''
        checks, failures, prgs = run_result(text, row['ok'], legacy)
        targets[row['target']] = {'status': 'pass' if row['ok'] else 'fail', 'checks': checks,
                                  'failures': failures, 'prg_sha256': prgs, 'commit': commit}
    piece = {'name': args.name, 'commit': commit, 'targets': targets}
    if args.certificates:
        piece['certificates'] = certificates()
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(piece, indent=1, sort_keys=True) + '\n')
    print(f'{args.out}: {len(targets)} vice targets, {sum(t["status"] == "pass" for t in targets.values())} passed')
    return 0


def cmd_merge(args):
    pieces = [json.loads(p.read_text()) for p in args.pieces]
    jobs = dict(job.split('=', 1) for job in args.job)
    main = next((p for p in pieces if 'certificates' in p), pieces[0] if pieces else {})
    targets, cover = {}, {}
    for piece in pieces:
        targets.update(piece['targets'])
        cover.update(piece.get('c64_cover', {}))   # (tools/cover_check.py pieces)
    manifest = {
        'schema': SCHEMA, 'commit': main.get('commit'), 'ref': os.environ.get('MANIFEST_REF', ''),
        'event': os.environ.get('GITHUB_EVENT_NAME', ''), 'run_id': os.environ.get('GITHUB_RUN_ID', ''),
        'run_attempt': os.environ.get('GITHUB_RUN_ATTEMPT', ''),
        'date': datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
        'complete': len(pieces) == args.expect and all(r in ('success', 'failure') for r in jobs.values()),
        'jobs': jobs, 'pieces': sorted(p['name'] for p in pieces),
        'targets': dict(sorted(targets.items())), 'certificates': main.get('certificates', {}),
        'c64_cover': dict(sorted(cover.items()))}
    args.out.write_text(json.dumps(manifest, indent=1) + '\n')
    passed = sum(t['status'] == 'pass' for t in targets.values())
    print(f'{args.out}: {passed}/{len(targets)} vice targets passed at {manifest["commit"]}, '
          f'complete={manifest["complete"]}')
    return 0


def token():
    for key in ('GH_TOKEN', 'GITHUB_TOKEN'):
        if os.environ.get(key):
            return os.environ[key]
    try:
        return subprocess.run(['gh', 'auth', 'token'], capture_output=True, text=True, timeout=10).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return ''


def api(url, auth):
    request = urllib.request.Request(url, headers={'Accept': 'application/vnd.github+json'})
    if auth:
        # Unredirected, so the token never follows the redirect to blob storage.
        request.add_unredirected_header('Authorization', f'Bearer {auth}')
    with urllib.request.urlopen(request, timeout=60) as response:
        return response.read()


def usable(manifest):
    return isinstance(manifest, dict) and manifest.get('schema') == SCHEMA and manifest.get('complete') and \
        manifest.get('ref') == BRANCH and manifest.get('event') in ('schedule', 'workflow_dispatch')


def cached():
    """The newest usable manifest under build/vice-manifest/, or None."""
    best = None
    for path in CACHE.glob('*.json'):
        try:
            manifest = json.loads(path.read_text())
        except (OSError, ValueError):
            continue
        if usable(manifest) and (best is None or manifest['date'] > best['date']):
            best = manifest
    return best


def fetch():
    """The newest complete nightly/dispatch manifest from GitHub, cached under
    build/vice-manifest/<artifact id>.json. Returns (manifest, None) or (None, why)."""
    sys.path.insert(0, str(ROOT / 'tools'))
    import evidence  # noqa: E402
    auth = token()
    try:
        listing = json.loads(api(f'https://api.github.com/repos/{evidence.repo_slug()}/actions/artifacts'
                                 f'?name={NAME}&per_page=30', auth))
        for artifact in listing.get('artifacts', []):
            if artifact.get('expired'):
                continue
            path = CACHE / f'{artifact["id"]}.json'
            if not path.is_file():
                with zipfile.ZipFile(io.BytesIO(api(artifact['archive_download_url'], auth))) as archive:
                    data = archive.read(f'{NAME}.json')
                CACHE.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
            manifest = json.loads(path.read_text())
            if usable(manifest):
                return manifest, None
        return None, f'no complete {NAME} artifact from a nightly or dispatched port-ci run on {BRANCH}'
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, urllib.error.URLError) as error:
        return None, f'{NAME} not fetched ({error}{"" if auth else "; no GH_TOKEN"})'


def latest(remote=False, path=None):
    """(manifest, why): a local file, else (with remote) GitHub, else the cache."""
    if path:
        return json.loads(pathlib.Path(path).read_text()), None
    why = None
    if remote:
        manifest, why = fetch()
        if manifest:
            return manifest, None
    manifest = cached()
    return (manifest, None) if manifest else (None, why or f'no {NAME} cached; run with --fetch or --manifest')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    piece = sub.add_parser('piece')
    piece.add_argument('--name', required=True)
    piece.add_argument('--summary', type=pathlib.Path, default=ROOT / 'build/ci-summary.json')
    piece.add_argument('--log-dir', type=pathlib.Path, default=ROOT / 'build/ci-logs')
    piece.add_argument('--certificates', action='store_true', help='record each certificate\'s vice_targets')
    piece.add_argument('--out', type=pathlib.Path, required=True)
    merge = sub.add_parser('merge')
    merge.add_argument('pieces', nargs='*', type=pathlib.Path)
    merge.add_argument('--job', action='append', default=[], help='NAME=RESULT of a VICE job')
    merge.add_argument('--expect', type=int, default=12, help='pieces of a complete run')
    merge.add_argument('--out', type=pathlib.Path, required=True)
    sub.add_parser('fetch')
    args = parser.parse_args()
    if args.command == 'piece':
        return cmd_piece(args)
    if args.command == 'merge':
        return cmd_merge(args)
    manifest, why = fetch()
    if not manifest:
        print('WARN:', why)
        return 1
    print(f'{NAME}: run {manifest["run_id"]} {manifest["date"]} commit {manifest["commit"]}, '
          f'{len(manifest["targets"])} targets')
    return 0


if __name__ == '__main__':
    sys.exit(main())
