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
import re
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


def pins_ok(entry, errors):
    """Each artifact a native_coverage entry pins must still hash to its pin
    (#20). A pin that went stale says so with pin_dropped instead."""
    for item in entry.get('native_coverage', []):
        for artifact in item.get('artifacts', []) if isinstance(item, dict) else []:
            if not isinstance(artifact, dict):
                continue
            label = entry['path'] + ': native_coverage artifact ' + str(artifact.get('path'))
            if 'sha256' not in artifact:
                if not artifact.get('pin_dropped'):
                    errors.append(label + ' has neither sha256 nor pin_dropped')
                continue
            candidate = (ROOT / artifact.get('path', '')).resolve()
            if ROOT not in candidate.parents or not candidate.is_file():
                errors.append(label + ' is missing')
            elif hashlib.sha256(candidate.read_bytes()).hexdigest() != artifact['sha256']:
                errors.append(label + ' no longer matches its pinned sha256 (re-pin it to evidence for the '
                              'current file, or replace the pin with pin_dropped and the reason)')


UNMAPPED = ROOT / 'docs/unmapped-native-sources.txt'
SOURCE_NAME = re.compile(r'src/[\w./+-]+\.(?:c|inc)')


def mapped_sources(files):
    """The src/ files some entry maps: its native_coverage native_files and
    the names its certificate's source-to-native map cites. None when a map
    is not on disk (run tools/evidence.py fetch --coverage first)."""
    found = set()
    for entry in files:
        for item in entry.get('native_coverage', []):
            if isinstance(item, dict):
                found.update(f for f in item.get('native_files', []) if isinstance(f, str))
        record = (((entry.get('complete_native_coverage') or {}).get('evidence') or {})
                  .get('source_to_native_map') or {})
        path = record.get('path', '')
        if path.endswith('.json'):
            if not (ROOT / path).is_file():
                return None
            found.update(SOURCE_NAME.findall((ROOT / path).read_text()))
    return found


def unmapped_ok(files, errors):
    """#22: every src/*.c and *.inc is mapped to an AGC file, or listed in
    docs/unmapped-native-sources.txt; the list only shrinks."""
    mapped = mapped_sources(files)
    if mapped is None:
        print('Unmapped native sources: not checked (a source-to-native map is not fetched)')
        return
    sources = {str(p.relative_to(ROOT)) for pattern in ('src/*.c', 'src/*.inc') for p in ROOT.glob(pattern)}
    listed = {line.split('#')[0].strip() for line in UNMAPPED.read_text().splitlines()} - {''}
    for path in sorted(sources - mapped - listed):
        errors.append(path + ' is in no native_files list or source-to-native map: map it, or list it in '
                      + str(UNMAPPED.relative_to(ROOT)) + ' as shared code (#22)')
    for path in sorted(listed - (sources - mapped)):
        errors.append(str(UNMAPPED.relative_to(ROOT)) + ' lists ' + path + ', which is '
                      + ('now mapped' if path in sources else 'gone') + ': remove the line')
    print(f'Unmapped native sources: {len(sources - mapped)} of {len(sources)} src/ files (#22)')


def runs_distinct(records, errors, label):
    """The comparison and both results categories must be their own records,
    not the source-to-native map cited again."""
    if not isinstance(records, dict):
        return
    source_map = (records.get('source_to_native_map') or {}).get('path')
    for category in FILE_EVIDENCE[1:]:
        if source_map and (records.get(category) or {}).get('path') == source_map:
            what = 'an independent comparison' if category == FILE_EVIDENCE[1] else 'a test run'
            errors.append(label + ': ' + category + ' points at the source-to-native map, not ' + what)


def audit(manifest, require_complete):
    errors = []
    if manifest.get("schema_version") != 1:
        errors.append("Unsupported inventory schema version")
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
        pins_ok(entry, errors)
        if expected_kind != 'agc':
            continue
        if entry.get('status') not in ('untranslated', 'partial', 'complete'):
            errors.append('Invalid coverage status: ' + path)
        if entry.get('status') == 'complete':
            certificate = entry.get('complete_native_coverage', {})
            if certificate.get('reviewed_all_source_paths') is not True or certificate.get('source_revision') != BASE:
                errors.append('Missing reviewed full-source certificate: ' + path)
            if certificate.get('qualification_kind') == 'documentation_preservation':
                # Verify original bytes directly. This exception cannot certify
                # an executable word, constant, allocation or other declaration.
                comment_only = all(not line.strip() or line.lstrip().startswith(b'#')
                                   for line in data.splitlines())
                if not comment_only:
                    errors.append('Documentation-only certificate contains executable/data/declaration source: ' + path)
                if certificate.get('algorithms_translated') != 0 or certificate.get('emitted_words') != 0:
                    errors.append('Documentation-only certificate must claim zero algorithms and emitted words: ' + path)
                evidence_ok(certificate.get('evidence'),
                            ('source_to_native_map', 'independent_assembly_comparison'), errors, path)
            else:
                evidence_ok(certificate.get('evidence'), FILE_EVIDENCE, errors, path)
                runs_distinct(certificate.get('evidence'), errors, path)
        elif require_complete:
            errors.append('Remaining native source coverage: ' + path)
    if require_complete:
        for program in TREES:
            application = manifest.get('integrated_applications', {}).get(program, {})
            if application.get('status') != 'complete' or application.get('reviewed_entire_program') is not True:
                errors.append('Integrated C64 application acceptance missing: ' + program)
            evidence_ok(application.get('evidence'), APPLICATION_EVIDENCE, errors, program)
    unmapped_ok(files, errors)
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
