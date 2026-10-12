#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Keep lane PRs off the shared files so parallel lanes never conflict.

  python3 tools/ci_guard.py --base <commit> [--remote]

Fails when a change edits a generated index, a frozen ledger, the frozen
Makefile or legacy evidence, or adds evidence outside its own milestone
manifest. Pipeline and steward branches (port/pipeline/*) own those files and
pass --allow-shared. With --remote it also checks that every changed
milestone's release-stored evidence is published.
"""
import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import evidence  # noqa: E402
import ledgers  # noqa: E402
import port_coverage  # noqa: E402

FROZEN = [
    (re.compile(r'^(README\.md|docs/(HANDOFF|ROADMAP|VERIFICATION|SOURCE_MAP)\.md)$'),
     'generated index: add docs/milestones/<id>.md instead (rebuilt after merge)'),
    (re.compile(r'^docs/ledgers/'), 'frozen pre-split ledger text'),
    (re.compile(r'^docs/milestones/legacy-'), 'migrated legacy milestone'),
    (re.compile(r'^Makefile$'), 'frozen Makefile: add tools/mk/<id>.mk instead'),
    (re.compile(r'^evidence/[^/]+$'), 'legacy flat evidence: use evidence/<id>/ and tools/evidence.py'),
]


def changes(base):
    out = subprocess.check_output(['git', 'diff', '--name-status', '--no-renames', f'{base}...HEAD',
                                   '--', '.'], cwd=ROOT, text=True)
    for line in out.splitlines():
        status, path = line.split('\t', 1)
        yield status, path[len('c64-port/'):] if path.startswith('c64-port/') else path


def certificates(text):
    entries = json.loads(text).get('files', []) if text else []
    return {(entry['path'], key): entry[key] for entry in entries
            for key in ('pending_complete_certificate', 'complete_native_coverage') if entry.get(key)}


def changed_certificates(base, errors):
    """A certificate a change adds or edits must cite separate comparison and
    run records, drafted or complete, so lanes find out here, not at review."""
    inventory = 'docs/port-coverage.json'
    before = subprocess.run(['git', 'show', f'{base}:c64-port/{inventory}'], cwd=ROOT,
                            capture_output=True, text=True).stdout
    old = certificates(before)
    for key, certificate in certificates((ROOT / inventory).read_text()).items():
        if old.get(key) != certificate:
            port_coverage.runs_distinct(certificate.get('evidence'), errors, f'{key[0]} ({key[1]})')


# include/rope_image.h reads any word of the rope; only AGC_BLOCK_TWO_SELF-CHECK
# (ROPECHK, STSHOSUM) may use it. Everything else reads constants through
# fixed_data.h's certified spans.
ROPE_IMAGE_USERS = {'src/self_check_body.inc', 'src/rope_image_host.c', 'platform/c64/rope_image_reu.c',
                    'tests/self_check_cases.c', 'tests/test_self_check.c', 'platform/host/self_check_snapshot.c'}


def rope_image_includes(errors):
    """Any source outside ROPE_IMAGE_USERS that includes rope_image.h."""
    listed = subprocess.run(['git', 'grep', '-l', '-E', r'#\s*include\s*[<"]rope_image\.h[">]', '--', '*.c', '*.h', '*.inc'],
                            cwd=ROOT, capture_output=True, text=True).stdout.split()
    for path in listed:
        if path not in ROPE_IMAGE_USERS:
            errors.append(f'{path}: includes rope_image.h, which only AGC_BLOCK_TWO_SELF-CHECK may use (read constants through fixed_data.h)')


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--base', required=True)
    parser.add_argument('--allow-shared', action='store_true')
    parser.add_argument('--remote', action='store_true')
    args = parser.parse_args()
    errors, ids = [], set()
    for status, path in changes(args.base):
        if not args.allow_shared:
            for pattern, why in FROZEN:
                if pattern.match(path):
                    errors.append(f'{path}: {why}')
        parts = path.split('/')
        if parts[0] == 'evidence' and len(parts) == 3 and status != 'D':
            ids.add(parts[1])
        elif parts[0] == 'evidence' and len(parts) > 3:
            errors.append(f'{path}: evidence/<id>/ is flat; no subdirectories')
    for milestone in sorted(ids):
        if not (ROOT / 'evidence' / milestone / evidence.MANIFEST).is_file():
            errors.append(f'evidence/{milestone}/: missing {evidence.MANIFEST}')
            continue
        records = json.loads((ROOT / 'evidence' / milestone / evidence.MANIFEST).read_text())['files']
        for path in (ROOT / 'evidence' / milestone).iterdir():
            tracked = subprocess.run(['git', 'ls-files', '--error-unmatch', str(path)], cwd=ROOT,
                                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
            if tracked and path.name not in (evidence.MANIFEST, '.gitignore') and \
                    records.get(path.name, {}).get('store') != 'tree':
                errors.append(f'evidence/{milestone}/{path.name}: committed but not a tree-stored manifest entry')
    changed_certificates(args.base, errors)
    rope_image_includes(errors)
    try:
        ledgers.load()
    except ValueError as error:
        errors.append(str(error))
    for error in errors:
        print('FAIL:', error, file=sys.stderr)
    if errors:
        return 1
    if ids:
        verify = [sys.executable, str(ROOT / 'tools/evidence.py'), 'verify', *sorted(ids)]
        if subprocess.run(verify + (['--remote'] if args.remote else []), cwd=ROOT).returncode:
            return 1
    print(f'PASS: no shared-ledger edits; {len(ids)} milestone evidence folders checked')
    return 0


if __name__ == '__main__':
    sys.exit(main())
