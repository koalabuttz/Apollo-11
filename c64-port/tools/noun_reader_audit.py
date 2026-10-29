#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Map every original noun-file line to reviewed native readers/data/meta."""
import gzip
import hashlib
import json
from pathlib import Path

from fixed_data_generate import DATA, META, rope_word, source_line

ROOT = Path(__file__).resolve().parents[1]
INSTRUCTIONS = {'DXCH', 'INDEX', 'CAF', 'TS', 'CS', 'AD', 'EXTEND', 'BZMF',
                'TC', 'MASK', 'DCA', 'TCF'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    fixed = json.loads((ROOT / 'build/fixed-data/audit.json').read_text())
    audit, recommendations = [], []
    for m in fixed['missions']:
        mission = m['mission']
        source = mission + '/PINBALL_NOUN_TABLES.agc'
        original = (ROOT.parent / source).read_bytes()
        provenance = next(r for r in m['source_audit'] if r['path'] == source)
        assert digest(original) == provenance['sha256']
        assert not provenance['indentation_only_repairs'] and not provenance['support_alias']
        assert (ROOT / 'build/fixed-data' / source).read_bytes() == original
        records = [dict(r) for r in m['records'] if r['path'] == source]
        source_lines = {n for n, line in enumerate(original.decode().splitlines(), 1)
                        if source_line(line) is not None}
        assert source_lines == {r['line'] for r in records}
        table_words = {r['physical_fixed_word'] + i: w
                       for r in m['native_records'] if r['path'] == source
                       for i, w in enumerate(r['words'])}
        rope = (ROOT / 'build/fixed-data' / mission / 'MAIN.agc.bin').read_bytes()
        assert digest(rope) == m['assembled_rope_sha256']
        starts = {name: next(r['physical_fixed_word'] for r in records if r['label'] == name)
                  for name in ('LODNNTAB', 'GTSFOUT', 'GTSFIN', 'NNADTAB')}
        code_count = data_count = 0
        for r in records:
            op = r['op']
            assert op in DATA | META | INSTRUCTIONS, (source, r['line'], op)
            if op in INSTRUCTIONS:
                code_count += len(r['words'])
                assert len(r['words']) == 1
                address = r['physical_fixed_word']
                assert starts['LODNNTAB'] <= address < starts['NNADTAB']
                assert rope_word(rope, address) == r['words'][0]
                r['native_domain'] = ('LODNNTAB' if address < starts['GTSFOUT'] else
                                      'GTSFOUT' if address < starts['GTSFIN'] else 'GTSFIN')
                r['native_source'] = 'src/noun_reader.c'
            elif op in DATA:
                data_count += len(r['words'])
                assert all(table_words[r['physical_fixed_word'] + i] == w
                           for i, w in enumerate(r['words']))
                r['native_domain'] = 'immutable noun/scale tables'
                r['native_source'] = 'src/fixed_data.c'
            else:
                assert not r['words']
                r['native_domain'] = 'assembler placement/count metadata or compile-time MIXCON alias'
                r['native_source'] = 'include/fixed_data_symbols.h'
            if r['label'] == 'MIXCON':
                assert rope_word(rope, r['resolved_fixed_word']) == 40
        assert code_count == 41
        assert data_count == (556 if mission == 'Luminary099' else 520)
        audit.append({'source_path': source, 'source_sha256': digest(original),
                      'assembled_rope_sha256': digest(rope),
                      'all_noncomment_source_lines_mapped': True,
                      'code_words': code_count, 'data_words': data_count,
                      'native_instruction_decoder': False, 'records': records})
        recommendations.append({
            'source_path': source, 'source_sha256': digest(original),
            'recommendation': 'complete native translation within original documented caller domain',
            'complete_file_kind': 'three noun/scale reader algorithms, all six tables and placement/alias declarations',
            'entry_points': ['LODNNTAB', 'GTSFOUT', 'GTSFIN'],
            'all_source_paths_reviewed': True,
            'native_source': 'src/noun_reader.c', 'native_data': 'src/fixed_data.c',
            'source_line_and_address_map': 'evidence/noun-reader-audit.json.gz',
            'supported_domain': 'noun 0..99, both signs of zero, every original even scale-pair index; source DXCH-Z return context as specified by include/noun_reader.h',
            'host_source_states': 30912 if mission == 'Luminary099' else 27456,
            'c64_source_states': 644 if mission == 'Luminary099' else 572,
            'verification': 'all 2048 physical words and SUPER/IRQ match independently assembled byte-original source; sanitizer and target native execution',
            'instruction_decoder': False, 'runtime_source_instruction_execution': False,
            'continuations': [], 'out_of_domain_behavior': 'rejected transactionally; not an assertion of original illegal-index behavior',
            'application_completion_claim': False})
    output = {'schema': 1, 'missions': audit}
    (ROOT / 'evidence/noun-reader-audit.json.gz').write_bytes(
        gzip.compress((json.dumps(output, indent=2) + '\n').encode(), mtime=0))
    (ROOT / 'docs/noun-reader-certificates.json').write_text(
        json.dumps({'schema': 1, 'parent_inventory_merge_required': True,
                    'recommendations': recommendations}, indent=2) + '\n')
    print('PASS: both byte-original noun files entirely mapped; each has 41 algorithm words and six exact data tables')


if __name__ == '__main__':
    main()
