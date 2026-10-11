#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Translate/audit original layout declarations, never executable source."""
import gzip
import hashlib
import json
from pathlib import Path
import re

from fixed_data_generate import source_line

ROOT = Path(__file__).resolve().parents[1]
FILES = {'Luminary099': ['ERASABLE_ASSIGNMENTS.agc', 'FLAGWORD_ASSIGNMENTS.agc',
                         'TAGS_FOR_RELATIVE_SETLOC.agc'],
         'Comanche055': ['ERASABLE_ASSIGNMENTS.agc', 'TAGS_FOR_RELATIVE_SETLOC.agc']}


def number(text):
    return int(text[:-1], 10) if text.endswith('D') else int(text, 8)


def address(kind, text):
    # Independently decode assembler symbol-table notation, rather than reuse
    # fixed_data_generate's symbol-address conversion or native addressing.
    if kind == 'C':
        return int(text, 8)
    if kind == 'E' and text.startswith('E'):
        bank, logical = text[1:].split(',')
        assert 0o1400 <= int(logical, 8) <= 0o1777
        return (int(bank, 8) << 8) | (int(logical, 8) & 0xff)
    if kind == 'F' and ',' in text:
        bank, logical = text.split(',')
        assert 0o2000 <= int(logical, 8) <= 0o3777
        return (int(bank, 8) << 10) | (int(logical, 8) & 0x3ff)
    return int(text, 8)


def rope_word(rope, physical):
    # Rope-file banks begin2,3,0,1; historical physical indexing is0,1,2,3.
    bank, offset = divmod(physical, 1024)
    file_bank = {0: 2, 1: 3, 2: 0, 3: 1}.get(bank, bank)
    pos = (file_bank * 1024 + offset) * 2
    return ((rope[pos] << 8) | rope[pos + 1]) // 2


def macro(prefix, kind, name):
    identifier = ''.join(c if c.isalnum() or c == '_' else '_%02x_' % ord(c) for c in name)
    return prefix + {'E': 'ERASABLE_WORD_', 'F': 'FIXED_WORD_', 'C': 'ASSEMBLER_VALUE_'}[kind] + identifier


def main():
    baseline = json.loads((ROOT / 'build/fixed-data/audit.json').read_text())
    audit, bindings, recommendations, metadata = [], [], [], {}
    for m in baseline['missions']:
        mission = m['mission']
        prefix = 'APOLLO_LM_' if mission == 'Luminary099' else 'APOLLO_CM_'
        work = ROOT / 'build/fixed-data' / mission
        listing = (work / 'assembly.lst').read_text()
        symbols = {name: (kind, value) for kind, name, value in
                   re.findall(r'\b\d+,([ECF]):\s+(\S+)\s+(\S+)', listing)}
        rope = (work / 'MAIN.agc.bin').read_bytes()
        assert hashlib.sha256(rope).hexdigest() == m['assembled_rope_sha256']
        fixed = []
        bugger = re.findall(r'Bugger word ([0-7]{5}) at ([0-7]{2}),([0-7]{4})\.', listing)
        assert len(bugger) == 36
        assert {int(b, 8) for _, b, _ in bugger} == set(range(36))
        for raw, bank, logical in bugger:
            b = int(bank, 8)
            p = address('F', bank + ',' + logical)
            v = int(raw, 8)
            assert rope_word(rope, p) == v
            # Independent signed magnitude sum modulo the14-bit mantissa;
            # this is checksum-data arithmetic, not an instruction engine.
            words = [rope_word(rope, b * 1024 + i) for i in range((p & 1023) + 1)]
            checksum = sum(-(w ^ 0o77777) if w & 0o40000 else w for w in words) % 0o37777
            assert checksum in (b, (-b) % 0o37777)
            fixed.append({'physical_word': p, 'value': v, 'role': 'BNKSUM bugger',
                          'bank': b, 'independent_signed_checksum': checksum})
        for file in FILES[mission]:
            source = mission + '/' + file
            original = (ROOT.parent / source).read_bytes()
            provenance = next(r for r in m['source_audit'] if r['path'] == source)
            assert hashlib.sha256(original).hexdigest() == provenance['sha256']
            lines = original.decode().splitlines()
            normalized = (work / provenance['assembler_filename']).read_text().splitlines()
            assert all(a.split() == b.split() for a, b in zip(lines, normalized))
            assert len(normalized) == len(lines) + (3 if provenance['support_alias'] else 0)
            if not provenance['indentation_only_repairs']:
                assert (work / provenance['assembler_filename']).read_bytes().startswith(original)
            rows = [dict(r) for r in m['records'] if r['path'] == source]
            assert {n for n, line in enumerate(lines, 1) if source_line(line)} == {r['line'] for r in rows}
            current = None
            fixed_current = None
            fixed_cursors = {bank: bank * 1024 for bank in range(36)}
            register_end = next((n for n, line in enumerate(lines, 1)
                                 if 'INTERPRETIVE REGISTERS ADDRESSED RELATIVE' in line), 0)
            for r in rows:
                op = r['op']
                assert op in {'EQUALS', '=', 'ERASE', 'SETLOC', 'BANK', 'BLOCK',
                              'BNKSUM', 'EBANK=', 'SBANK=', 'MEMORY', 'COUNT', 'ECADR', '-16'}
                if r['label'] in symbols:
                    kind, segmented = symbols[r['label']]
                    value = address(kind, segmented)
                    assert value == (r['resolved_erasable_word'] if kind == 'E' else
                                     r['resolved_fixed_word'] if kind == 'F' else
                                     int(r['resolved_symbol'], 8))
                    cls = {'E': 1, 'F': 2, 'C': 3}[kind]
                    if kind == 'C' and file == 'ERASABLE_ASSIGNMENTS.agc' and r['line'] < register_end:
                        cls = 4  # explicitly original physical special-register declarations
                    bindings.append({'macro': macro(prefix, kind, r['label']), 'value': value,
                                     'kind': cls, 'source_path': source, 'source_line': r['line']})
                if op == 'SETLOC':
                    current = number(r['arguments'])
                    assert current == r['physical_erasable_word']
                    r['native_semantics'] = 'compile-time canonical allocation cursor'
                elif op == 'ERASE':
                    arg = r['arguments']
                    if ' - ' in arg:
                        # Original absolute-range syntax defines an alias;
                        # the assembled current location does not advance.
                        first, last = (number(x) for x in arg.split(' - '))
                        assert r['resolved_erasable_word'] == first
                        r['declared_absolute_range'] = [first, last]
                        r['allocation_words'] = 0
                        r['native_semantics'] = 'absolute alias; allocation cursor preserved'
                    else:
                        assert current == r['physical_erasable_word'], (source, r['line'], current)
                        if r['label']:
                            assert current == r['resolved_erasable_word']
                        count = 1 + (number(arg.lstrip('+')) if arg else 0)
                        r['allocation_start'] = current
                        r['allocation_words'] = count
                        current += count
                        assert current <= 2048
                        r['native_semantics'] = 'canonical physical word interval, aliases share image'
                elif op in ('=', 'EQUALS'):
                    parts = r['arguments'].split()
                    if parts:
                        if parts[0] in symbols:
                            k, text = symbols[parts[0]]
                            computed = address(k, text)
                        else:
                            computed = number(parts[0])
                        if len(parts) == 2:
                            computed += number(parts[1])
                        assert len(parts) <= 2
                        if r['label']:
                            assert computed == bindings[-1]['value'], (source, r['line'], computed, bindings[-1])
                        r['independent_expression_value'] = computed
                    else:
                        assert r['label'] and bindings[-1]['value'] == (
                            fixed_current if file == 'TAGS_FOR_RELATIVE_SETLOC.agc' else current)
                    r['native_semantics'] = 'compile-time original typed alias/value; no storage duplication'
                elif op == '-16':
                    assert mission == 'Comanche055' and r['line'] == 2094
                    assert current == 0o1356 and r['words'] == [0o21356]
                    r['native_semantics'] = 'retain literal erasable emission and one-word allocation advance'
                    r['allocation_start'] = current
                    r['allocation_words'] = 1
                    current += 1
                elif op == 'ECADR':
                    kind, segmented = symbols[r['arguments']]
                    assert kind == 'E'
                    assert r['words'] == [address(kind, segmented)]
                    assert rope_word(rope, r['physical_fixed_word']) == r['words'][0]
                    assert r['physical_fixed_word'] == fixed_current
                    fixed_current += 1
                    fixed_cursors[(fixed_current - 1) // 1024] = fixed_current
                    fixed.append({'physical_word': r['physical_fixed_word'], 'value': r['words'][0],
                                  'role': 'original CM EB-switch data', 'source_line': r['line']})
                    r['native_semantics'] = 'immutable native original EB-switch word'
                elif op == 'MEMORY':
                    assert r['arguments'] == '120000 - 167777'
                    r['unavailable_pseudo_range'] = [0o120000, 0o167777]
                    r['unavailable_physical_banks'] = [36, 55]
                    r['native_semantics'] = 'exclude original unavailable fixed banks; no native RAM allocation'
                elif op == 'BNKSUM':
                    assert number(r['arguments']) in range(36)
                    r['native_semantics'] = 'retained original independent-assembly bugger metadata'
                elif op in ('BANK', 'BLOCK'):
                    bank = number(r['arguments'])
                    fixed_current = fixed_cursors[bank]
                    assert fixed_current == r['physical_fixed_word']
                    r['independent_fixed_cursor'] = fixed_current
                    r['native_semantics'] = 'compile-time physical fixed-bank tag cursor'
                elif op == 'EBANK=':
                    kind, segmented = symbols[r['arguments']]
                    assert kind == 'E'
                    r['source_eb_register'] = (address(kind, segmented) // 256) * 256
                    r['native_semantics'] = 'source compile-time ECADR bank selection metadata'
                elif op == 'SBANK=':
                    kind, segmented = symbols[r['arguments']]
                    assert kind == 'F'
                    r['source_superbank_enable'] = 0o100 if address(kind, segmented) // 1024 >= 32 else 0
                    r['native_semantics'] = 'source compile-time fixed superbank tag metadata'
                else:
                    r['native_semantics'] = 'assembler placement/latch/count metadata only'
            audit.append({'source_path': source, 'source_sha256': provenance['sha256'],
                          'original_lines': len(lines), 'all_declaration_lines_mapped': True,
                          'indentation_only_repairs': provenance['indentation_only_repairs'],
                          'support_alias': provenance['support_alias'], 'records': rows})
            recommendations.append({'source_path': source, 'source_sha256': provenance['sha256'],
                'recommendation': 'complete original declaration/layout translation',
                'complete_file_kind': 'typed aliases, allocation intervals and original declaration metadata/data; no executable algorithm',
                'all_source_paths_reviewed': True, 'native_source': 'src/layout.c',
                'native_bindings': 'include/fixed_data_symbols.h',
                'source_line_and_address_map': 'evidence/layout-audit.json.gz',
                'instruction_decoder': False, 'runtime_source_instruction_execution': False,
                'startup_initialization_claim': False, 'continuations': []})
        metadata[mission] = sorted(fixed, key=lambda r: r['physical_word'])
    assert len(bindings) == 4232 and len({b['macro'] for b in bindings}) == 4232
    assert len(metadata['Luminary099']) == 36 and len(metadata['Comanche055']) == 43
    native = ['/* SPDX-License-Identifier: MIT; generated original layout metadata. */']
    for mission, suffix in [('Luminary099', 'lm'), ('Comanche055', 'cm')]:
        rows = metadata[mission]
        native.append('const apollo_layout_fixed_word apollo_layout_%s_fixed[%d] = {' % (suffix, len(rows)))
        native.extend('    {0%06o,0%06o},' % (r['physical_word'], r['value']) for r in rows)
        native.append('};')
    native.append('const apollo_layout_fixed_word apollo_layout_cm_erasable_emission = {01356,021356};')
    (ROOT / 'src/layout_data.inc').write_text('\n'.join(native) + '\n')
    vectors = ['/* SPDX-License-Identifier: MIT; independently assembled original expectations. */',
               '#ifndef LAYOUT_VECTORS_H', '#define LAYOUT_VECTORS_H',
               '#include "layout.h"', '#define LAYOUT_BINDINGS 4232u',
               'static const uint16_t layout_native_bindings[LAYOUT_BINDINGS] = {']
    vectors.extend('    %s,' % b['macro'] for b in bindings)
    vectors.extend(['};', 'static const uint16_t layout_expected[LAYOUT_BINDINGS] = {'])
    for i in range(0, len(bindings), 16):
        vectors.append('    ' + ','.join('0%06o' % b['value'] for b in bindings[i:i+16]) + ',')
    vectors.extend(['};', 'static const uint8_t layout_kind[LAYOUT_BINDINGS] = {'])
    for i in range(0, len(bindings), 32):
        vectors.append('    ' + ','.join(str(b['kind']) for b in bindings[i:i+32]) + ',')
    vectors.extend(['};', '#define LAYOUT_METADATA_WORDS 79u',
                    'static const uint16_t layout_metadata_address[79] = {'])
    all_fixed = metadata['Luminary099'] + metadata['Comanche055']
    vectors.append(','.join(str(r['physical_word']) for r in all_fixed))
    vectors.extend(['};', 'static const uint16_t layout_metadata_value[79] = {'])
    vectors.append(','.join('0%06o' % r['value'] for r in all_fixed))
    vectors.extend(['};', '#endif'])
    (ROOT / 'tests/layout_vectors.h').write_text('\n'.join(vectors) + '\n')
    manifest = {'schema': 1, 'runtime_instruction_decoder': False, 'files': audit,
                'bindings': bindings, 'fixed_metadata': metadata,
                'source_support_qualification': 'byte-original declarations atop independently assembled pinned support; support algorithms not certified',
                'cm_original_vs_pinned': next(m for m in baseline['missions'] if m['mission'] == 'Comanche055')['symbol_changes_against_pinned_support'],
                'cm_original_only_symbols': next(m for m in baseline['missions'] if m['mission'] == 'Comanche055')['original_only_symbols'],
                'cm_pinned_only_symbols': next(m for m in baseline['missions'] if m['mission'] == 'Comanche055')['pinned_only_symbols'],
                'cm_affected_support_words': next(m for m in baseline['missions'] if m['mission'] == 'Comanche055')['baseline_differences']}
    (ROOT / 'evidence/layout-audit.json.gz').write_bytes(gzip.compress(
        (json.dumps(manifest, indent=2) + '\n').encode(), mtime=0))
    (ROOT / 'docs/layout-certificates.json').write_text(json.dumps(
        {'schema': 1, 'parent_inventory_merge_required': True, 'recommendations': recommendations}, indent=2) + '\n')
    print('PASS: all five original declaration files; 4232 typed bindings, 79 fixed metadata words and one CM erasable emission')


if __name__ == '__main__':
    main()
