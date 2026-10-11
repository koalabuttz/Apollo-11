#!/usr/bin/env python3
"""Audit the complete LM+CM native-port inventory and immutable historical source.

Passing --require-complete requires reviewed coverage certificates and integrated
application evidence. File existence and hashes support review; this tool does
not prove flight correctness or turn a partial primitive into a complete port.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
REPOSITORY = ROOT.parent
BASE = '911e5c0283c629c50cb97666f34065e8c07d71a5'
TREES = ('Luminary099', 'Comanche055')
FILE_EVIDENCE = ('source_to_native_map', 'independent_assembly_comparison',
                 'host_and_sanitizer_results', 'c64_execution_results')
APPLICATION_EVIDENCE = ('bootable_c64_application', 'integrated_program_tests',
                        'device_adapter_tests', 'native_output_tests',
                        'memory_and_overlay_validation', 'recovery_and_restart_tests')


def baseline():
    paths = subprocess.check_output(
        ['git', 'ls-tree', '-r', '--name-only', BASE, '--', *TREES],
        cwd=REPOSITORY, text=True).splitlines()
    requests = ''.join(f'{BASE}:{path}\n' for path in paths).encode()
    data = subprocess.run(['git', 'cat-file', '--batch'], cwd=REPOSITORY,
                          input=requests, stdout=subprocess.PIPE, check=True).stdout
    contents = {}
    cursor = 0
    for path in paths:
        end = data.index(b'\n', cursor)
        fields = data[cursor:end].split()
        if len(fields) != 3 or fields[1] != b'blob':
            raise ValueError('Missing original blob: ' + path)
        size = int(fields[2])
        cursor = end + 1
        contents[path] = data[cursor:cursor + size]
        cursor += size + 1
    return contents


def evidence_ok(records, required, errors, label):
    if not isinstance(records, dict):
        errors.append(label + ': missing evidence dictionary')
        return
    for category in required:
        record = records.get(category)
        if not isinstance(record, dict):
            errors.append(label + ': missing ' + category)
            continue
        path = record.get('path', '')
        candidate = (ROOT / path).resolve()
        if not path or ROOT not in candidate.parents or not candidate.is_file():
            errors.append(label + ': missing/outside evidence file for ' + category)
            continue
        digest = hashlib.sha256(candidate.read_bytes()).hexdigest()
        if digest != record.get('sha256'):
            errors.append(label + ': evidence SHA mismatch for ' + category)


def audit(manifest, require_complete):
    errors = []
    original = baseline()
    files = manifest.get('files', [])
    entries = {entry['path']: entry for entry in files}
    if len(entries) != len(files) or set(entries) != set(original):
        errors.append('Inventory paths differ from the complete original source trees')
    actual = {str(path.relative_to(REPOSITORY)) for tree in TREES
              for path in (REPOSITORY / tree).rglob('*') if path.is_file()}
    if actual != set(original):
        errors.append('Historical trees have missing or additional files')
    if manifest.get('source_baseline', {}).get('revision') != BASE:
        errors.append('Inventory does not use the pinned immutable original revision')
    if manifest.get('source_baseline', {}).get('trees') != list(TREES):
        errors.append('Both complete historical program trees are required')
    constraints = manifest.get('target_constraints', {})
    if constraints.get('language') != 'native C' or constraints.get('agc_instruction_decoder') is not False:
        errors.append('Completion target must be native C without an AGC instruction decoder')
    if constraints.get('program_subset') is not False:
        errors.append('Completion scope must include both entire programs')
    for path, data in original.items():
        entry = entries.get(path)
        current = REPOSITORY / path
        if not current.is_file() or current.read_bytes() != data:
            errors.append('Historical source changed: ' + path)
        if entry is None:
            continue
        if entry.get('sha256') != hashlib.sha256(data).hexdigest() or entry.get('lines') != len(data.splitlines()):
            errors.append('Inventory source hash/line count mismatch: ' + path)
        if entry.get('bytes') != len(data):
            errors.append('Inventory source byte count mismatch: ' + path)
        expected_kind = 'agc' if path.endswith('.agc') else 'source_documentation'
        if entry.get('kind') != expected_kind:
            errors.append('Wrong inventory file kind: ' + path)
        if expected_kind != 'agc':
            continue
        if entry.get('status') not in ('untranslated', 'partial', 'complete'):
            errors.append('Invalid coverage status: ' + path)
        if entry.get('status') == 'complete':
            certificate = entry.get('complete_native_coverage', {})
            if certificate.get('reviewed_all_source_paths') is not True or certificate.get('source_revision') != BASE:
                errors.append('Missing reviewed full-source certificate: ' + path)
            evidence_ok(certificate.get('evidence'), FILE_EVIDENCE, errors, path)
        elif require_complete:
            errors.append('Remaining native source coverage: ' + path)
    if require_complete:
        for program in TREES:
            application = manifest.get('integrated_applications', {}).get(program, {})
            if application.get('status') != 'complete' or application.get('reviewed_entire_program') is not True:
                errors.append('Integrated C64 application acceptance missing: ' + program)
            evidence_ok(application.get('evidence'), APPLICATION_EVIDENCE, errors, program)
    for tree in TREES:
        source = [entry for entry in files if entry.get('kind') == 'agc' and entry['path'].startswith(tree + '/')]
        counts = {status: sum(entry.get('status') == status for entry in source)
                  for status in ('untranslated', 'partial', 'complete')}
        print(f'{tree}: {len(source)} AGC files, {sum(entry["lines"] for entry in source):,} source lines; '
              f'{counts["untranslated"]} untranslated, {counts["partial"]} partial, {counts["complete"]} complete')
    remaining = [entry['path'] for entry in files
                 if entry.get('kind') == 'agc' and entry.get('status') != 'complete']
    print(f'Remaining files requiring complete native coverage: {len(remaining)}')
    print('Historical-source baseline:', BASE)
    return errors, remaining


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=pathlib.Path, default=ROOT / 'docs/port-coverage.json')
    parser.add_argument('--require-complete', action='store_true')
    parser.add_argument('--list-remaining', action='store_true')
    args = parser.parse_args()
    try:
        manifest = json.loads(args.inventory.read_text())
        errors, remaining = audit(manifest, args.require_complete)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print('FAIL:', error, file=sys.stderr)
        return 1
    if args.list_remaining:
        for path in remaining:
            print(path)
    if errors:
        for error in errors:
            print('FAIL:', error, file=sys.stderr)
        return 1
    print('PASS: inventory and immutable source checks' + ('; complete LM+CM native C64 acceptance' if args.require_complete else ' (port completion is not certified)'))
    return 0


if __name__ == '__main__':
    sys.exit(main())
