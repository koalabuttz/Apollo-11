#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 TRIM_GIMBAL_CONTROL_SYSTEM
(src/lm_trim_gimbal.c), checked by tools/lm_basic_source.py. Writes
build/lm-trim-gimbal-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

OWNERS = [(0o21, 0o3077, 0o3326, 'gts'), (0o16, 0o3735, 0o3771, 'acdtc12'),
          (0o21, 0o3327, 0o3432, 'timegmbl'), (0o21, 0o3433, 0o3433, 'timegmbl'),
          (0o21, 0o3434, 0o3434, 'lm_interbank_sites'), (0o21, 0o3435, 0o3437, 'timeret'),
          (0o21, 0o3440, 0o3440, 'timegmbl'), (0o21, 0o3441, 0o3523, 'gtsqrt'), (0o21, 0o3524, 0o3635, 'gts'),
          (0o21, 0o3636, 0o3645, 'rootcycl')]
HELPERS = {'gts': ['gtsqrt', 'wand', 'dtcb', 'sv', 'sa', 'at', 'bit15'], 'gtsqrt': ['rootcycl', 'sv', 'bit15'],
           'rootcycl': [], 'acdtc12': ['rand', 'wr'], 'timegmbl': ['sv', 'at']}
# Branches composed in C: GTS's labels and its GTSQRT call (RSTOFGTS),
# GTSQRT's labels and ROOTCYCL calls, TIMEGMBL's labels, and the TC
# FUNCTION +1 return.
INLINE = {'gts': {0o3116, 0o3130, 0o3576, 0o3200, 0o3203, 0o3220, 0o3222, 0o3230, 0o3227, 0o3232, 0o3233,
                  0o3247, 0o3240, 0o3244, 0o3265, 0o3303, 0o3316, 0o3524, 0o3566, 0o3323, 0o3537, 0o3562,
                  0o3563, 0o3573, 0o3572, 0o3575, 0o3621, 0o3616, 0o3610, 0o3115, 0o3441},
          'gtsqrt': {0o3460, 0o3445, 0o3451, 0o3454, 0o3461, 0o3473, 0o3507, 0o3465, 0o3467, 0o3636, 0o1752},
          'timegmbl': {0o3413, 0o3363, 0o3417, 0o3371, 0o3373, 0o3425, 0o3427, 0o3336, 0o3337},
          'acdtc12': {0o3752, 0o3757}}
CHANNELS = {0o12: 'chan12'}
SITES = {'lm_interbank_sites': 'src/lm_interbank_sites.inc'}


def main():
    result, owned = check('src/lm_trim_gimbal.c', {'TRIM_GIMBAL_CONTROL_SYSTEM.agc': None}, (0o21, 0o16), OWNERS,
                          HELPERS, INLINE, CHANNELS, SITES, work_name='trim-gimbal-source')
    result['note'] = ('Fresh yaYUL assembly of the file (with the indentation-only repair of its +2 location field) '
                      'reproduces the certified rope; every assembled word of TRIM_GIMBAL_CONTROL_SYSTEM has one '
                      'native owner holding its erasable address (the indexed base for INDEX orders), the rope '
                      'value of its fixed operand, its branch target (or composes it inline), its channel or its '
                      'constant; the IBNKCALL CADR is the compiled site operand.')
    (R / 'build/lm-trim-gimbal-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
