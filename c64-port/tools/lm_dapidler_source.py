#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 DAPIDLER_PROGRAM
(src/lm_dapidler.c), checked by tools/lm_basic_source.py. Writes
build/lm-dapidler-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

OWNERS = [(0o16, 0o2000, 0o2015, 'chekbits_body'), (0o16, 0o2016, 0o2023, 'chekmore'),
          (0o16, 0o2024, 0o2036, 'dapidler_body'), (0o16, 0o2037, 0o2040, 'lm_exec_sites'),
          (0o16, 0o2041, 0o2041, 'checkup_body'), (0o16, 0o2042, 0o2046, 'accsokay'),
          (0o16, 0o2047, 0o2047, 'lm_interbank_sites'), (0o16, 0o2050, 0o2151, 'startdap'),
          (0o16, 0o2152, 0o2154, 'settime5'), (0o16, 0o2155, 0o2156, 'shutdown'),
          (0o16, 0o2157, 0o2157, 'moreidle'), (0o16, 0o2160, 0o2160, 'lm_interbank_sites'),
          (0o16, 0o2161, 0o2161, 'calcperr'), (0o16, 0o2162, 0o2162, 'lm_interbank_sites'),
          (0o16, 0o2163, 0o2201, 'shutdown'), (0o16, 0o2202, 0o2202, 'startdap'), (0o16, 0o2203, 0o2203, 'shutdown'),
          (0o16, 0o2204, 0o2205, 'startdap'), (0o16, 0o2206, 0o2212, 'chekbits_body'),
          (0o20, 0o2226, 0o2434, 'altdsply'), (0o20, 0o2435, 0o2441, 'oversub2'), (0o20, 0o2442, 0o2446, 'retnmore')]
HELPERS = {'chekbits_body': ['moreidle', 'shutdown', 'dtcb', 'rd'], 'moreidle': ['transfer'],
           'shutdown': ['settime5', 'wr', 'wand'], 'chekmore': ['moreidle', 'dapidler_body', 'rand'],
           'dapidler_body': ['checkup_body', 'transfer'], 'checkup_body': ['chekbits_body'],
           'accsokay': ['moreidle', 'transfer'], 'startdap': ['settime5'], 'calcperr': ['transfer'],
           'altdsply': ['oversub2', 'retnmore', 'msu', 'rxor_l', 'wor', 'wand', 'rand', 'transfer', 'sa'],
           'oversub2': ['sa'], 'retnmore': ['dtcb']}
# Branches composed in C: CHEKBITS's MOREIDLE, JUMPDSP and SHUTDOWN, the
# CHECKUP and CHEKBITS calls, SETTIME5, and ALTDSPLY's labels, TC OVERSUB2
# and TCF RETNMORE.
INLINE = {'chekbits_body': {0o2157, 0o2206, 0o2163}, 'chekmore': {0o2157}, 'dapidler_body': {0o2041},
          'checkup_body': {0o2000}, 'accsokay': {0o2157}, 'shutdown': {0o2152},
          'altdsply': {0o2322, 0o2252, 0o2442, 0o2435, 0o2362, 0o2353, 0o2373, 0o2406, 0o2414, 0o2374}}
CHANNELS = {0o1: 'rxor_l', 0o5: 'chan5', 0o6: 'chan6', 0o12: 'chan12', 0o14: 'chan14', 0o30: 'chan30',
            0o31: 'chan31'}
SITES = {'lm_exec_sites': 'src/lm_exec_sites.inc', 'lm_interbank_sites': 'src/lm_interbank_sites.inc'}


def main():
    result, owned = check('src/lm_dapidler.c', {'DAPIDLER_PROGRAM.agc': None}, (0o16, 0o20), OWNERS, HELPERS, INLINE,
                          CHANNELS, SITES, work_name='dapidler-source')
    result['note'] = ('Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every assembled '
                      'word of DAPIDLER_PROGRAM has one native owner holding its erasable address (the indexed base '
                      'for INDEX orders), the rope value of its fixed operand, its branch target (or composes it '
                      'inline), its channel or its constant; the NOVAC 2CADR and the IBNKCALL CADRs are the '
                      'compiled site operands.')
    (R / 'build/lm-dapidler-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
