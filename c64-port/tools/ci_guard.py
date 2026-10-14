#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Keep lane PRs off the shared files so parallel lanes never conflict.

  python3 tools/ci_guard.py --base <commit> [--remote]

Fails when a change edits a generated index, a frozen ledger, the frozen
Makefile or legacy evidence, adds evidence outside its own milestone
manifest, or adds or edits a coverage certificate that tools/cert_precheck.py
FAILs. Pipeline and steward branches (port/pipeline/*) own those files and
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
import cert_precheck  # noqa: E402
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


def changed_certificates(base, errors, remote=False):
    """A certificate a change adds or edits must cite separate comparison and
    run records, drafted or complete, and pass tools/cert_precheck.py, so lanes
    find out here, not at review."""
    inventory = 'docs/port-coverage.json'
    # Compare with the merge base, as changes() does, so a branch behind the
    # base is not charged with certificates the base changed since.
    fork = subprocess.run(['git', 'merge-base', base, 'HEAD'], cwd=ROOT,
                          capture_output=True, text=True).stdout.strip() or base
    before = subprocess.run(['git', 'show', f'{fork}:c64-port/{inventory}'], cwd=ROOT,
                            capture_output=True, text=True).stdout
    old = certificates(before)
    changed = []
    for key, certificate in certificates((ROOT / inventory).read_text()).items():
        if old.get(key) != certificate:
            port_coverage.runs_distinct(certificate.get('evidence'), errors, f'{key[0]} ({key[1]})')
            changed.append(key[0])
    if changed:
        report = cert_precheck.precheck(list(dict.fromkeys(changed)), cert_precheck.Report(),
                                        notes=False, fetch_missing=remote)
        for line in report.lines:
            if line.startswith('FAIL: ') and 'points at the source-to-native map' not in line:
                errors.append('cert_precheck: ' + line[len('FAIL: '):])
            elif line.startswith('WARN: '):
                print(line)


# include/rope_image.h reads any word of the rope; only AGC_BLOCK_TWO_SELF-CHECK
# (ROPECHK, STSHOSUM) may use it. Everything else reads constants through
# fixed_data.h's certified spans. tools/flight_host_run.c (host only, beside
# yaAGC) loads the image for the self-check module of the whole program.
# The runtime interpreter (INTERPRETER.agc, decided 2026-10-13) runs the
# interpretive programs as the rope data they are, so it reads the rope too.
# main-assembly checks MAIN.agc's include manifest against every rope word.
# INTERPRETER's compiled basic subroutines (lm/cm-interpreter) read their
# callers' words (DMP's ADRES, POLY's coefficients) as the core does.
ROPE_IMAGE_USERS = {'src/lm_interpreter.c', 'src/cm_interpreter.c',
                    'tests/lm_interpreter_target.c', 'tests/cm_interpreter_target.c',
                    'src/lm_interp_runtime.c', 'src/lm_interp_core.c', 'tools/interp_core_fuzz.c', 'tools/interp_core_cycles.c',
                    'tools/interp_runtime_target.c', 'tools/interp_core_vice.c', 'tools/interp_flight_replay.c',
                    'tools/interp_flight_verify.c',
                    'src/self_check_body.inc', 'src/rope_image_host.c', 'platform/c64/rope_image_reu.c',
                    'tests/self_check_cases.c', 'tests/test_self_check.c', 'platform/host/self_check_snapshot.c',
                    'tools/flight_host_run.c', 'platform/c64/flight_table_module.c',
                    'tests/main_manifest_check.c', 'tests/test_main_manifest.c'}


def rope_image_includes(errors):
    """Any source outside ROPE_IMAGE_USERS that includes rope_image.h."""
    listed = subprocess.run(['git', 'grep', '-l', '-E', r'#\s*include\s*[<"]rope_image\.h[">]', '--', '*.c', '*.h', '*.inc'],
                            cwd=ROOT, capture_output=True, text=True).stdout.split()
    for path in listed:
        if path not in ROPE_IMAGE_USERS:
            errors.append(f'{path}: includes rope_image.h, which only AGC_BLOCK_TWO_SELF-CHECK may use (read constants through fixed_data.h)')


# Comanche055 symbols whose value in the byte-original declaration rope
# (6e3150ae..., include/fixed_data_symbols.h) comes from an upstream
# transcription error (docs/FIXED_DATA.md, "Upstream errata"). Native code
# must use the flown value, which Virtual AGC's pinned rope (the oracle) has.
ERRATA_SYMBOLS = {
    'APOLLO_CM_ERASABLE_WORD_ALTVAR': 'ALTVAR is 01356 in the flown rope; 01357 comes from a stray -16 at ERASABLE_ASSIGNMENTS.agc:2094',
    'APOLLO_CM_FIXED_WORD_R53FLBIT': 'R53FLBIT is BIT6 in the flown rope; BIT8 is an upstream transcription error',
}


def errata_symbols(errors):
    """Native sources that use a symbol whose generated value is an upstream erratum."""
    for symbol, why in ERRATA_SYMBOLS.items():
        listed = subprocess.run(['git', 'grep', '-l', '-w', symbol, '--', 'src', 'platform'],
                                cwd=ROOT, capture_output=True, text=True).stdout.split()
        for path in listed:
            errors.append(f'{path}: uses {symbol}; {why} (docs/FIXED_DATA.md, "Upstream errata")')


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
    changed_certificates(args.base, errors, args.remote)
    rope_image_includes(errors)
    errata_symbols(errors)
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
