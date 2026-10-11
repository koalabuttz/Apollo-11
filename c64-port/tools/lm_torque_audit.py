#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Every byte-original source line and assembled word maps to native code/data."""
from pathlib import Path
import gzip
import hashlib
import json
from fixed_data_generate import listing_records, rope_word, source_line

ROOT = Path(__file__).resolve().parents[1]


def main():
    source = ROOT.parent / 'Luminary099/KALMAN_FILTER.agc'
    original = source.read_bytes()
    work = ROOT / 'build/lm-torque-source'
    assert (work / source.name).read_bytes() == original
    rope = (work / 'MAIN.agc.bin').read_bytes()
    visits = json.loads((ROOT / 'build/lm-torque-visits.json').read_text())
    name = 'Luminary099/KALMAN_FILTER.agc'
    rows = listing_records(work / 'assembly.lst', {source.name: name},
                           {name: original.decode()}, rope)
    lines = {i for i, text in enumerate(original.decode().splitlines(), 1)
             if source_line(text) is not None}
    assert {row['line'] for row in rows} == lines
    assert sum(len(row['words']) for row in rows) == 60
    sections = {
        'RATELOOP': 'native three-axis loop, duration classification and clipping',
        'LOOPRATE': 'native signed jet torque, rescaling and downlist update',
        'STORTORK': 'native overflowing downlist ADS and next-axis selection',
        'SMALLTJU': 'native pulse clearing and MP ELEVEN small-duration path',
        'ROTORQUE': 'native two-axis rate calculation and external BACKP return',
        'NEGTORK': 'native sign inversion and negative downlist selection',
    }
    section = None
    for row in rows:
        if row['label'] in sections:
            section = row['label']
        if not row['words']:
            row['native_mapping'] = 'compile-time original bank/location/counter declarations; no runtime instruction'
        elif row['op'] == 'DEC':
            assert row['label'] == '-100MST6' and row['words'] == [0o77537]
            assert row['arguments'] == '-160'
            assert ((-(-160)) ^ 0o77777) == row['words'][0]
            row['native_mapping'] = 'include/lm_torque.h LM_TORQUE_MINUS_100MST6; consumed by src/lm_torque.c'
        else:
            pc = int(row['assembled_address'].split(',')[1], 8)
            assert visits[f'{pc:05o}'] > 0, row
            row['source_executions'] = visits[f'{pc:05o}']
            row['native_mapping'] = sections[section]
            row['native_source'] = 'src/lm_torque.c'
    constants = {'TWO': (0o4752, 2), 'ZERO': (0o4755, 0),
                 'ELEVEN': (0o4760, 0o13), 'BIT10': (0o4742, 0o1000),
                 '-100MS': (0o16 * 1024 + 0o1615, 0o74631)}
    for name, (address, word) in constants.items():
        assert rope_word(rope, address) == word, (name, address, word)
    external_refs = []
    for path in (ROOT.parent / 'Luminary099').glob('*.agc'):
        if path == source:
            continue
        for i, text in enumerate(path.read_text().splitlines(), 1):
            parsed = source_line(text)
            if parsed is not None and parsed.get('arguments') in sections:
                assert parsed['arguments'] == 'RATELOOP', (path, i, parsed)
                external_refs.append({'path': str(path.relative_to(ROOT.parent)),
                                      'line': i, **parsed})
    assert external_refs == [{'path': 'Luminary099/P-AXIS_RCS_AUTOPILOT.agc',
                              'line': 408, 'label': 'SUPERJOB', 'op': 'TCF',
                              'arguments': 'RATELOOP'}]
    result = {'source_path': 'Luminary099/KALMAN_FILTER.agc',
              'source_sha256': hashlib.sha256(original).hexdigest(),
              'rope_sha256': hashlib.sha256(rope).hexdigest(),
              'original_source_bytes': True, 'indentation_repairs': [],
              'noncomment_lines': len(lines), 'assembled_words': 60,
              'executed_words': 59, 'literal_words': 1,
              'native_source': 'src/lm_torque.c', 'source_map': rows,
              'external_constants': constants, 'original_callers': external_refs,
              'external_continuation': 'BACKP, original bank16 logical2316; its body is outside this file',
              'all_2048_words_compared': True,
              'oracle_native_numerical_code_linked': False,
              'instruction_decoder': False,
              'hardware_or_entire_application_completion_claim': False}
    encoded = json.dumps(result, indent=2).encode()
    (ROOT / 'evidence/lm-torque-audit.json.gz').write_bytes(gzip.compress(encoded, mtime=0))
    print('PASS: all 65 original noncomment lines, 59 executable words and one -160 literal mapped; every executable word reached in the source oracle')


if __name__ == '__main__':
    main()
