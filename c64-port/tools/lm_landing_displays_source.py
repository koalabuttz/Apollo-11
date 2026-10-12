#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 LANDING_ANALOG_DISPLAYS
(src/lm_landing_displays.c), checked by tools/lm_basic_source.py. Writes
build/lm-landing-displays-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

B = 0o21
OWNERS = [(B, 0o2154, 0o2362, 'flow'), (B, 0o2363, 0o2363, 'lm_landing_displays_sites'), (B, 0o2364, 0o3076, 'flow')]
HELPERS = {'flow': ['oc', 'rand', 'wor', 'wand', 'leave', 'jump', 'tc_ladqsave']}
# Branches composed in C: the transfers among this file's own words (its
# labels, and the TC DISINDAT calls with their returns).
INLINE = {'flow': {0o2231, 0o2233, 0o2254, 0o2270, 0o2272, 0o2273, 0o2313, 0o2320, 0o2324, 0o2374, 0o2503, 0o2620, 0o2627, 0o2635, 0o2643, 0o2651, 0o2656, 0o2662, 0o2667, 0o2674, 0o2701, 0o2703, 0o2712, 0o2716, 0o2720, 0o2730, 0o2735, 0o2742, 0o2761, 0o2766, 0o2772, 0o2775, 0o3020, 0o3024, 0o3027, 0o3053, 0o3067}}
CHANNELS = {0o12: 'chan12', 0o14: 'chan14', 0o30: 'chan30'}
SITES = {'lm_landing_displays_sites': 'src/lm_landing_displays_sites.inc'}


def main():
    result, owned = check('src/lm_landing_displays.c', {'LANDING_ANALOG_DISPLAYS.agc': None}, B, OWNERS, HELPERS,
                          INLINE, CHANNELS, SITES, work_name='landing-displays-source')
    result['note'] = ('Fresh yaYUL assembly of the file reproduces the certified rope; every assembled word of '
                      'LANDING_ANALOG_DISPLAYS (bank 21) has one native owner holding its erasable address (the '
                      'indexed base for INDEX orders), the rope value of its fixed operand, its branch target (or '
                      'composes it inline), its channel or its constant; the ADRES INTLZE operand of TC TWIDDLE '
                      'is a compiled site operand.')
    (R / 'build/lm-landing-displays-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
