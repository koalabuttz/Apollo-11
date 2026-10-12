#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 THROTTLE_CONTROL_ROUTINES
and LUNAR_LANDING_GUIDANCE_EQUATIONS FASTCHNG (src/lm_throttle.c), checked by
tools/lm_basic_source.py. Writes build/lm-throttle-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

OWNERS = [(0o2216, 0o2220, 'throttle'), (0o2221, 0o2223, 'throttle3_body'), (0o2224, 0o2227, 'present'),
          (0o2230, 0o2302, 'desired'), (0o2303, 0o2311, 'pif'), (0o2312, 0o2367, 'doit'),
          (0o2370, 0o2370, 'flatout'), (0o2371, 0o2377, 'whatout_body'), (0o2400, 0o2416, 'massmult'),
          (0o2417, 0o2417, 'throttle'), (0o3740, 0o3745, 'fastchng')]
HELPERS = {'throttle': ['throttle3_body'], 'throttle3': ['throttle3_body'], 'throttle3_body': ['transfer'],
           'present': ['transfer'], 'desired': ['transfer'], 'pif': ['doit'], 'doit': ['tc_indirect'],
           'flatout': ['whatout_body'], 'whatout_body': ['doit'], 'massmult': ['dmp', 'tpagree', 'tc_indirect']}
# Targets composed inline: the branches inside desired, doit and massmult,
# WHATOUT's TCF DOIT, and the INTERPRETER subroutines DMP (07103) and
# TPAGREE (07257) that MASSMULT compiles at its call sites.
INLINE = {'desired': {0o2244, 0o2272, 0o2267, 0o2277, 0o2301, 0o2302},
          'doit': {0o2335, 0o2336, 0o2361},
          'whatout_body': {0o2312},
          'massmult': {0o7103, 0o7257, 0o2415}}
CHANNELS = {0o14: 'chan14'}


def main():
    result, owned = check('src/lm_throttle.c',
                          {'THROTTLE_CONTROL_ROUTINES.agc': None,
                           'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc': [(0o3740, 0o3745)]},
                          0o31, OWNERS, HELPERS, INLINE, CHANNELS, work_name='throttle-source')
    result['note'] = ('Fresh yaYUL assembly of the unchanged files reproduces the certified rope; every assembled '
                      'word of THROTTLE_CONTROL_ROUTINES and of FASTCHNG has one native owner holding its erasable '
                      'address, the rope value of its fixed operand, its branch target (or composes it inline), its '
                      'channel or its constant.')
    (R / 'build/lm-throttle-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
