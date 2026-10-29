#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 TJET_LAW
(src/lm_tjet_law.c), checked by tools/lm_basic_source.py. Writes
build/lm-tjet-law-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

OWNERS = [(0o3252, 0o3525, 'tjetlaw'), (0o3526, 0o3541, 'z123comp'), (0o3542, 0o3711, 'tjetlaw'),
          (0o3712, 0o3720, 'rufsetup'), (0o3721, 0o3743, 'tjetlaw')]
HELPERS = {'tjetlaw': ['z123comp', 'rufsetup', 'xcell', 'ix', 'transfer'], 'z123comp': ['xcell']}
# TJETLAW is one routine: every branch inside the file is a C label or
# condition there (the TC Z123COMP and TC RUFSETUP / RUFSETUP +2 calls are
# the helpers z123comp and rufsetup), and Z123COMP's BZMF +2 (RETURN) and
# TCF FULLTIME are its two exits.
INLINE = {'tjetlaw': {0o3266, 0o3276, 0o3306, 0o3320, 0o3333, 0o3340, 0o3344, 0o3347, 0o3414, 0o3422, 0o3424,
                      0o3426, 0o3434, 0o3465, 0o3466, 0o3502, 0o3503, 0o3513, 0o3526, 0o3542, 0o3547, 0o3555,
                      0o3562, 0o3616, 0o3623, 0o3630, 0o3636, 0o3645, 0o3654, 0o3661, 0o3664, 0o3672, 0o3712,
                      0o3714},
          'z123comp': {0o3465, 0o3541}}


def main():
    result, owned = check('src/lm_tjet_law.c', {'TJET_LAW.agc': None}, 0o17, OWNERS, HELPERS, INLINE,
                          work_name='tjet-law-source')
    result['note'] = ('Fresh yaYUL assembly of the unchanged file reproduces the certified rope; every assembled '
                      'word of TJET_LAW has one native owner holding its erasable address (the indexed base for '
                      'INDEX orders), the rope value of its fixed operand, its branch target (or composes it '
                      'inline), or its constant.')
    (R / 'build/lm-tjet-law-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
