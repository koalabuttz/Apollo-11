#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Translate original constant declarations into native immutable data.

yaYUL is a build-time independent assembler, never linked into the native port.
Every emitted native word is checked against both its source-line listing and
the binary rope. Placement/declaration files are audited, not executable ports.
"""
import argparse
import hashlib
import json
import os
import gzip
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PIN = '1b5d6474841a7ab53237ead070f76f35f162f68d'
NOUNS = ('NNADTAB', 'NNTYPTAB', 'SFINTAB', 'SFOUTAB', 'IDADDTAB', 'RUTMXTAB')
DATA = {'OCT', 'DEC', '2OCT', '2DEC', 'DEC*', '2DEC*', 'ECADR', 'FCADR',
        'GENADR', 'ADRES', 'CADR', '2CADR', 'BBCON', 'BBCON*', 'VN', 'DNPTR',
        'DNCHAN', '1DNADR', '2DNADR', '3DNADR', '4DNADR', '5DNADR', '6DNADR'}
DATA |= {'-' + op for op in DATA}
META = {'EBANK=', 'SBANK=', 'BANK', 'BLOCK', 'SETLOC', 'COUNT', 'COUNT*',
        'EQUALS', 'EQUALS2', '=', '=MINUS', 'CHECK=', 'ERASE', 'MEMORY', 'SUBRO'}
EMITTED = {
    'Luminary099': ('CONTROLLED_CONSTANTS.agc', 'FIXED_FIXED_CONSTANT_POOL.agc',
                    'INTERPRETIVE_CONSTANT.agc', 'DOWNLINK_LISTS.agc',
                    'RESTART_TABLES.agc'),
    'Comanche055': ('FIXED_FIXED_CONSTANT_POOL.agc', 'INTERPRETIVE_CONSTANTS.agc',
                   'STAR_TABLES.agc', 'DOWNLINK_LISTS.agc'),
}
MIXED_PREFIXES = {'Comanche055': {'RESTART_TABLES.agc': 'NEWPHASE'},
                  'Luminary099': {}}
DECLARATIONS = {
    'Luminary099': ('ERASABLE_ASSIGNMENTS.agc', 'FLAGWORD_ASSIGNMENTS.agc',
                    'TAGS_FOR_RELATIVE_SETLOC.agc'),
    'Comanche055': ('ERASABLE_ASSIGNMENTS.agc', 'TAGS_FOR_RELATIVE_SETLOC.agc'),
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def source_line(line):
    text = line.split('#', 1)[0].rstrip()
    if not text:
        return None
    if text.startswith('$'):
        return {'include': text[1:].split()[0]}
    fields = text.split()
    if text[0].isspace():
        return {'label': '', 'op': fields[0], 'arguments': ' '.join(fields[1:])}
    return {'label': fields[0], 'op': fields[1] if len(fields) > 1 else '',
            'arguments': ' '.join(fields[2:])}


def data_record(record):
    if record['op'] in DATA:
        return True
    # TC has a zero opcode field here and is explicitly used as a numeric
    # expression assembler for the twelve documented SIZETAB offset entries.
    # These words are read as data; no TC instruction is executed or decoded.
    return (record['op'] == 'TC' and record['path'].endswith('/RESTART_TABLES.agc')
            and record['line'] in (
                range(94, 106) if record['path'].startswith('Luminary099/')
                else range(101, 113)))


def inventory(mission):
    result = []
    for path in sorted((ROOT.parent / mission).glob('*.agc')):
        ops = {}
        for line in path.read_text().splitlines():
            parsed = source_line(line)
            if parsed is not None and 'op' in parsed:
                ops[parsed['op']] = ops.get(parsed['op'], 0) + 1
        unknown = sorted(set(ops) - DATA - META - {'BNKSUM'})
        if mission == 'Luminary099' and path.name == 'RESTART_TABLES.agc':
            assert ops.get('TC') == 12
            unknown.remove('TC')
        # A source continuation in the CM ERASE layout occupies operand columns,
        # not the opcode column. Only this explicit reviewed layout file is
        # classified by its actual assembly records below.
        if path.name in DECLARATIONS[mission]:
            kind = ('source_layout_with_non_data_line' if unknown else
                    'source_layout_or_alias_declarations')
        elif path.name in EMITTED[mission]:
            assert not unknown, (path, unknown)
            kind = 'complete_constant_declarations'
        elif not ops:
            kind = 'include_manifest' if path.name == 'MAIN.agc' else 'comments_only'
        elif not unknown:
            kind = 'other_data_or_layout'
        else:
            kind = 'mixed_or_executable'
        result.append({'path': str(path.relative_to(ROOT.parent)),
                       'sha256': sha(path.read_bytes()), 'kind': kind,
                       'op_counts': ops, 'non_data_ops': unknown})
    return result


def fixed_address(text):
    text = text.strip()
    match = re.fullmatch(r'([0-7]{2}),([0-7]{4})', text)
    if match:
        bank, address = (int(value, 8) for value in match.groups())
        assert 0 <= bank < 36 and 0o2000 <= address < 0o4000, text
        return bank * 1024 + address - 0o2000
    if re.fullmatch('[0-7]{4}', text) and 0o4000 <= int(text, 8) < 0o10000:
        return int(text, 8)
    return None


def rope_word(rope, physical):
    bank, offset = divmod(physical, 1024)
    file_bank = bank ^ 2 if bank < 4 else bank
    position = 2 * (file_bank * 1024 + offset)
    return int.from_bytes(rope[position:position + 2], 'big') >> 1


def physical_erasable(text):
    match = re.fullmatch(r'E([0-7]),([0-7]{4})', text.strip())
    if match:
        bank, address = (int(value, 8) for value in match.groups())
        if 0o1400 <= address < 0o2000:
            return bank * 256 + address - 0o1400
    if re.fullmatch('[0-7]{4,7}', text.strip()) and int(text, 8) < 0o1400:
        return int(text, 8)
    return None


def symbols(listing):
    return dict(re.findall(r'\b\d+,[ECF]:\s+(\S+)\s+(\S+)', listing))


def symbol_types(listing):
    return {name: kind for kind, name in
            re.findall(r'\b\d+,([ECF]):\s+(\S+)\s+\S+', listing)}


def source_listing(listing):
    current, last_label = 'MAIN.agc', ''
    records = []
    for text in listing.splitlines():
        if text.startswith('(End of include-file '):
            current, last_label = 'MAIN.agc', ''
            continue
        match = re.match(r'^\d{6},(\d{6}): (.*)$', text)
        if not match:
            continue
        if match[2].startswith('$'):
            current, last_label = match[2][1:].split()[0], ''
            continue
        label = text[45:65].strip()
        if label:
            last_label = label
        records.append({'assembler_filename': current, 'line': int(match[1]),
                        'address': text[14:23].strip(), 'label': label,
                        'preceding_label': last_label,
                        'operation': text[65:74].strip(),
                        'operands': text[74:].split('#', 1)[0].strip(),
                        'listing': text})
    return records


def filename_alias(original, includes):
    if original in includes:
        return original
    candidates = [name for name in includes
                  if name.replace('-', '_').startswith(original[:-4])]
    assert len(candidates) == 1, (original, candidates)
    return candidates[0]


def listing_records(path, aliases, originals, rope):
    current = 'MAIN.agc'
    result = []
    for text in path.read_text().splitlines():
        if text.startswith('(End of include-file '):
            current = 'MAIN.agc'
            continue
        match = re.match(r'^\d{6},(\d{6}): (.*)$', text)
        if not match:
            continue
        if match[2].startswith('$'):
            current = match[2][1:].split()[0]
            continue
        original = aliases.get(current)
        if original is None:
            continue
        number = int(match[1])
        lines = originals[original].splitlines()
        if number > len(lines):
            continue  # Documented zero-word support alias.
        parsed = source_line(lines[number - 1])
        if parsed is None or 'op' not in parsed:
            continue
        address = text[14:23].strip()
        words = text[33:44].split()
        words = [int(word, 8) for word in words if re.fullmatch('[0-7]{5}', word)]
        physical = fixed_address(address)
        if words and physical is None:
            assert original.endswith('/ERASABLE_ASSIGNMENTS.agc'), (
                original, number, address, words)
        if words and physical is not None:
            for offset, word in enumerate(words):
                assert rope_word(rope, physical + offset) == word, (
                    original, number, address, word, rope_word(rope, physical + offset))
        result.append({'path': original, 'line': number, **parsed,
                       'assembled_address': address, 'physical_fixed_word': physical,
                       'physical_erasable_word': physical_erasable(address),
                       'words': words})
    return result


def numeric_source_checks(records):
    checked = 0
    for record in records:
        if not record['words']:
            continue
        op, argument = record['op'], record['arguments']
        if op in ('OCT', '2OCT'):
            pieces = argument.split()
            values = []
            for value in pieces:
                assert re.fullmatch(r'[+-]?[0-7]+', value), (record, value)
                magnitude = int(value.lstrip('+-'), 8)
                mask = 0o7777777777 if op == '2OCT' and len(pieces) == 1 else 0o77777
                encoded = magnitude ^ (mask if value.startswith('-') else 0)
                if op == '2OCT' and len(pieces) == 1:
                    values.extend([encoded >> 15, encoded & 0o77777])
                else:
                    values.append(encoded)
            assert values == record['words'], record
            checked += len(values)
    return checked


def make_segments(records):
    segments = []
    for record in records:
        if not record['words']:
            continue
        address = record['physical_fixed_word']
        assert address is not None
        if segments and segments[-1]['path'] == record['path'] and (
                segments[-1]['address'] + len(segments[-1]['words']) == address):
            segment = segments[-1]
        else:
            segment = {'path': record['path'], 'address': address, 'words': []}
            segments.append(segment)
        segment['words'].extend(record['words'])
    occupied = {}
    for segment in segments:
        for offset, word in enumerate(segment['words']):
            address = segment['address'] + offset
            assert address not in occupied, ('overlapping data declarations', address)
            occupied[address] = word
    return segments


def generate_native(mission, segments, nouns, source_digest, output):
    key = 'lm' if mission == 'Luminary099' else 'cm'
    lines = ['/* SPDX-License-Identifier: MIT */',
             '/* Generated by tools/fixed_data_generate.py; original public-domain data.',
             f' * Mission {mission}; source-set SHA256 {source_digest}. */']
    for index, segment in enumerate(segments):
        lines.append(f'/* {segment["path"]}; historical fixed word {segment["address"]:06o}. */')
        lines.append(f'static const uint16_t fixed_data_{key}_{index}[] = {{')
        words = segment['words']
        for start in range(0, len(words), 12):
            lines.append('    ' + ', '.join(f'0{word:05o}' for word in words[start:start + 12]) + ',')
        lines.append('};')
    lines.append(f'static const fixed_span fixed_data_{key}_spans[] = {{')
    for index, segment in enumerate(segments):
        lines.append(f'    {{ {segment["address"]}, {len(segment["words"])}, fixed_data_{key}_{index} }},')
    lines.append('};')
    lines.append(f'static const apollo_fixed_table fixed_data_{key}_nouns[] = {{')
    for noun in nouns:
        lines.append(f'    {{ {noun["address"]}, {noun["word_count"]} }}, /* {noun["name"]} */')
    lines.append('};')
    output.write_text('\n'.join(lines) + '\n')


def assemble(mission, output_root):
    base = ROOT / ('deps/fixture' if mission == 'Luminary099' else 'deps/cm-services-fixture')
    assert (base / 'MAIN.agc.bin').exists(), f'Prepare existing {mission} oracle fixture first'
    # Assemble in a private directory, then move each file into place with
    # os.replace: parallel test recipes read build/fixed-data/<mission> while
    # another recipe re-assembles it, and must never find a file missing.
    final = output_root / mission
    work = output_root / f'.{mission}.tmp-{os.getpid()}'
    shutil.rmtree(work, ignore_errors=True)
    work.mkdir(parents=True)
    for path in base.glob('*.agc'):
        shutil.copyfile(path, work / path.name)
    includes = re.findall(r'^\$(\S+)', (work / 'MAIN.agc').read_text(), re.M)
    selected = (EMITTED[mission] + DECLARATIONS[mission] +
                tuple(MIXED_PREFIXES[mission]) + ('PINBALL_NOUN_TABLES.agc',))
    aliases, originals, source_audit = {}, {}, []
    for name in selected:
        original = ROOT.parent / mission / name
        data = original.read_bytes()
        text = data.decode()
        original_name = str(original.relative_to(ROOT.parent))
        originals[original_name] = text
        alias = filename_alias(name, includes)
        aliases[alias] = original_name
        normalized = re.sub(r'^\t([+-]\d+D?)\t', r' \1\t\t', text, flags=re.M)
        normalized = re.sub(r'^([+-]\d+D?)\t', r' \1\t', normalized, flags=re.M)
        repairs = [number for number, (a, b) in enumerate(
                   zip(text.splitlines(), normalized.splitlines()), 1) if a != b]
        if name == 'ERASABLE_ASSIGNMENTS.agc' and mission == 'Luminary099':
            normalized += '\n# Zero-word support alias, preserving original TTPIO.\nTTPI0 EQUALS TTPIO\n'
        (work / alias).write_text(normalized)
        source_audit.append({'path': original_name, 'sha256': sha(data),
                             'assembler_filename': alias, 'indentation_only_repairs': repairs,
                             'support_alias': 'TTPI0 EQUALS original TTPIO' if
                             name == 'ERASABLE_ASSIGNMENTS.agc' and mission == 'Luminary099' else None})
    with (work / 'assembly.lst').open('w') as log:
        subprocess.run([str(ROOT / 'deps/virtualagc/yaYUL/yaYUL'), 'MAIN.agc'],
                       cwd=work, stdout=log, stderr=subprocess.STDOUT, check=True)
    final.mkdir(parents=True, exist_ok=True)
    for path in work.iterdir():
        os.replace(path, final / path.name)
    work.rmdir()
    work = final
    rope = (work / 'MAIN.agc.bin').read_bytes()
    records = listing_records(work / 'assembly.lst', aliases, originals, rope)
    selected_data = [record for record in records
                     if Path(record['path']).name in EMITTED[mission] and record['words']]
    for record in selected_data:
        assert data_record(record), record
    coverage = []
    for name in EMITTED[mission]:
        path = mission + '/' + name
        source_rows = {number: source_line(line) for number, line in
                       enumerate(originals[path].splitlines(), 1)}
        expected = {number for number, row in source_rows.items()
                    if row is not None and 'op' in row and
                    data_record({'path': path, 'line': number, **row})}
        actual = {row['line'] for row in selected_data if row['path'] == path}
        assert expected == actual, (path, expected - actual, actual - expected)
        coverage.append({'path': path, 'emitted_source_lines': len(expected),
                         'all_declared_data_lines_mapped': True,
                         'native_words': sum(len(row['words']) for row in selected_data
                                             if row['path'] == path)})
    mixed_data_prefixes = []
    for name, stop_label in MIXED_PREFIXES[mission].items():
        path = mission + '/' + name
        stop = next(record['line'] for record in records
                    if record['path'] == path and record['label'] == stop_label)
        prefix = [record for record in records
                  if record['path'] == path and record['line'] < stop and record['words']]
        for record in prefix:
            assert data_record(record), record
        selected_data.extend(prefix)
        mixed_data_prefixes.append({'path': path, 'before_executable_label': stop_label,
                                    'stop_line': stop,
                                    'native_words': sum(len(record['words']) for record in prefix)})
    noun_records = [record for record in records
                    if record['path'].endswith('/PINBALL_NOUN_TABLES.agc')]
    nouns, noun_words = [], []
    for name in NOUNS:
        first = next(record for record in noun_records if record['label'] == name)
        start = noun_records.index(first)
        words = []
        rows = []
        for record in noun_records[start:]:
            if record is not first and record['label'] in NOUNS:
                break
            if record['words']:
                assert record['op'] in DATA, record
                assert record['physical_fixed_word'] == first['physical_fixed_word'] + len(words)
                words.extend(record['words'])
                rows.append(record)
        nouns.append({'name': name, 'address': first['physical_fixed_word'],
                      'assembled_address': first['assembled_address'], 'word_count': len(words)})
        noun_words.extend(rows)
    assert [item['word_count'] for item in nouns][:2] == [100, 100]
    assert nouns[4]['word_count'] == 180 and nouns[5]['word_count'] == 60
    selected_data += sorted(noun_words, key=lambda row: row['line'])
    octal_words = numeric_source_checks(selected_data)
    segments = make_segments(selected_data)
    source_digest = sha(json.dumps(source_audit, sort_keys=True).encode())
    generate_native(mission, segments, nouns, source_digest,
                    ROOT / 'src' / ('fixed_data_lm.inc' if mission == 'Luminary099' else 'fixed_data_cm.inc'))
    baseline = (base / 'MAIN.agc.bin').read_bytes()
    listing = (work / 'assembly.lst').read_text()
    full_records = source_listing(listing)
    original_symbols = symbols(listing)
    pinned_symbols = symbols((base / 'assembly.lst').read_text())
    original_types = symbol_types(listing)
    pinned_types = symbol_types((base / 'assembly.lst').read_text())
    for record in records:
        resolved = original_symbols.get(record['label'])
        kind = original_types.get(record['label'])
        record['resolved_symbol'] = resolved
        record['resolved_symbol_kind'] = kind
        record['resolved_fixed_word'] = fixed_address(resolved) if kind == 'F' else None
        record['resolved_erasable_word'] = physical_erasable(resolved) if kind == 'E' else None
    symbol_changes = []
    for name in sorted(original_symbols.keys() & pinned_symbols.keys()):
        if original_symbols[name] != pinned_symbols[name]:
            symbol_changes.append({'name': name, 'original': original_symbols[name],
                                   'pinned': pinned_symbols[name],
                                   'kind': original_types[name],
                                   'physical_erasable_original':
                                   physical_erasable(original_symbols[name])
                                   if original_types[name] == 'E' else None,
                                   'physical_erasable_pinned':
                                   physical_erasable(pinned_symbols[name])
                                   if pinned_types[name] == 'E' else None,
                                   'original_fixed_value': rope_word(rope, fixed_address(
                                       original_symbols[name])) if original_types[name] == 'F' else None,
                                   'pinned_fixed_value': rope_word(baseline, fixed_address(
                                       pinned_symbols[name])) if pinned_types[name] == 'F' else None,
                                   'references': [row for row in full_records
                                                  if re.search(r'(?<![\w.])' + re.escape(name) +
                                                               r'(?![\w.])', row['operands'])]})
    differences = []
    for physical in range(36 * 1024):
        a, b = rope_word(rope, physical), rope_word(baseline, physical)
        if a != b:
            bank, offset = divmod(physical, 1024)
            address = f'{bank:02o},{0o2000 + offset:04o}'
            # Record the full independently assembled line for changed support
            # words, including its source opcode/operand; do not certify it.
            support_rows = [row for row in full_records if row['address'] == address]
            differences.append({'address': address, 'physical_fixed_word': physical,
                                'original_overlay_word': a, 'pinned_support_word': b,
                                'assembly_records': support_rows,
                                'role': 'bank checksum' if not support_rows else 'support instruction or data'})
    return {'mission': mission, 'source_files': inventory(mission),
            'source_audit': source_audit, 'source_set_sha256': source_digest,
            'complete_data_file_coverage': coverage,
            'mixed_file_data_prefixes': mixed_data_prefixes,
            'symbol_changes_against_pinned_support': symbol_changes,
            'original_only_symbols': sorted(original_symbols.keys() - pinned_symbols.keys()),
            'pinned_only_symbols': sorted(pinned_symbols.keys() - original_symbols.keys()),
            'layout_emitted_non_data_words': [row for row in records if row['words'] and
                                            row['physical_fixed_word'] is None],
            'assembled_rope_sha256': sha(rope), 'baseline_rope_sha256': sha(baseline),
            'baseline_differences': differences, 'records': records,
            'native_records': selected_data, 'native_segments': segments, 'noun_tables': nouns,
            'octal_source_words_independently_checked': octal_words,
            'native_word_count': sum(len(segment['words']) for segment in segments),
            'scope': 'Complete listed constant-declaration files; six Pinball data sections only. '
                     'Layout/alias/comment files audited separately. No executable path certificate.'}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / 'build/fixed-data')
    args = parser.parse_args()
    actual = subprocess.check_output(['git', '-C', str(ROOT / 'deps/virtualagc'),
                                      'rev-parse', 'HEAD'], text=True).strip()
    assert actual == PIN
    missions = [assemble(mission, args.output) for mission in EMITTED]
    manifest = {'schema': 1, 'assembler_source_pin': PIN,
                'native_instruction_decoder': False, 'missions': missions}
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / 'audit.json').write_text(json.dumps(manifest, indent=2) + '\n')
    evidence = ROOT / 'evidence/fixed-data-audit.json.gz'
    evidence.write_bytes(gzip.compress(json.dumps(manifest, sort_keys=True).encode(),
                                       mtime=0))
    certificates = []
    symbol_header = ['/* SPDX-License-Identifier: MIT */',
                     '/* Generated original source label/alias bindings; no storage. */',
                     '#ifndef APOLLO_FIXED_DATA_SYMBOLS_H',
                     '#define APOLLO_FIXED_DATA_SYMBOLS_H']
    seen = {}
    for mission in missions:
        prefix = 'APOLLO_LM_' if mission['mission'] == 'Luminary099' else 'APOLLO_CM_'
        for row in mission['records']:
            if row['resolved_symbol'] is None or not row['label']:
                continue
            identifier = ''.join(character if character.isalnum() or character == '_' else
                                 f'_{ord(character):02x}_' for character in row['label'])
            if row['resolved_symbol_kind'] == 'F':
                category, value = 'FIXED_WORD_', row['resolved_fixed_word']
            elif row['resolved_symbol_kind'] == 'E':
                category, value = 'ERASABLE_WORD_', row['resolved_erasable_word']
            elif re.fullmatch(r'[+-]?[0-7]+', row['resolved_symbol']):
                category, value = 'ASSEMBLER_VALUE_', int(row['resolved_symbol'], 8)
            else:
                continue
            assert value is not None, row
            macro = prefix + category + identifier
            if macro in seen:
                assert seen[macro] == (row['label'], value), (macro, row)
                continue
            seen[macro] = (row['label'], value)
            symbol_header.append(f'#define {macro} ({value}) /* {row["label"]}; '
                                 f'{row["path"]}:{row["line"]} */')
        hashes = {row['path']: row['sha256'] for row in mission['source_audit']}
        for file in mission['complete_data_file_coverage']:
            certificates.append({
                'source_path': file['path'], 'source_sha256': hashes[file['path']],
                'recommendation': 'complete immutable data translation',
                'complete_file_kind': 'constant and alias declarations, no executable algorithm',
                'all_declared_data_lines_mapped': file['all_declared_data_lines_mapped'],
                'native_words': file['native_words'],
                'native_source': 'src/fixed_data.c',
                'native_data': 'src/fixed_data_lm.inc' if mission['mission'] == 'Luminary099'
                               else 'src/fixed_data_cm.inc',
                'source_line_and_address_map': 'evidence/fixed-data-audit.json.gz',
                'verification': 'each original declaration assembled; every raw word checked '
                                'against listing, external rope, host API and native C64 API',
                'continuations': [], 'instruction_decoder': False,
                'runtime_source_instruction_execution': False})
    (ROOT / 'docs/fixed-data-certificates.json').write_text(
        json.dumps({'schema': 1, 'parent_inventory_merge_required': True,
                    'recommendations': certificates}, indent=2) + '\n')
    symbol_header.append('#endif')
    (ROOT / 'include/fixed_data_symbols.h').write_text('\n'.join(symbol_header) + '\n')
    for mission in missions:
        print(f'{mission["mission"]}: {mission["native_word_count"]} exact native data words; '
              f'{len(mission["native_segments"])} spans; six noun tables')
        print('Noun tables:', [(n['name'], n['assembled_address'], n['word_count'])
                                for n in mission['noun_tables']])
        print('Baseline differences:', mission['baseline_differences'])
    print(args.output / 'audit.json')


if __name__ == '__main__':
    main()
