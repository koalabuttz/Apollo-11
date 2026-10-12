#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 Q_R-AXIS_RCS_AUTOPILOT
(src/lm_q_r_axis.c), checked by tools/lm_basic_source.py. Writes
build/lm-q-r-axis-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

B = 0o17
OWNERS = [(B, 0o2072, 0o2170, 'flow'), (B, 0o2171, 0o2171, 'lm_q_r_axis_sites'), (B, 0o2172, 0o2212, 'flow'),
          (B, 0o2213, 0o2213, 'lm_interbank_sites'), (B, 0o2214, 0o2342, 'flow'), (B, 0o2343, 0o2343, 'qr_data'),
          (B, 0o2344, 0o2452, 'flow'), (B, 0o2453, 0o2453, 'lm_interbank_sites'), (B, 0o2454, 0o2570, 'flow'),
          (B, 0o2571, 0o2572, 'qr_data'), (B, 0o2573, 0o2616, 'flow'), (B, 0o2617, 0o2617, 'lm_interbank_sites'),
          (B, 0o2620, 0o2625, 'flow'), (B, 0o2626, 0o2660, 'qerrcalc_body'), (B, 0o2661, 0o2722, 'flow'),
          (B, 0o2723, 0o2723, 'lm_interbank_sites'), (B, 0o2724, 0o3110, 'flow'), (B, 0o3111, 0o3112, 'qr_data'),
          (B, 0o3113, 0o3113, 'flow'), (B, 0o3114, 0o3145, 'jtlst_body'), (B, 0o3146, 0o3172, 'rot_touv_body'),
          (B, 0o3173, 0o3222, 'selctsub'), (B, 0o3223, 0o3223, 'lm_q_r_axis_sites'), (B, 0o3224, 0o3224, 'flow'),
          (B, 0o3225, 0o3235, 'selctsub'), (B, 0o3236, 0o3251, 'flow'), (3, 0o7754, 0o7755, 'flow')]
HELPERS = {'qerrcalc_body': ['msu', 'tcq'], 'rot_touv_body': ['tcq'], 'selctsub': ['tcq', 'sv'],
           'jtlst_body': ['wor13', 'tcq', 'rot_touv_body'],
           'flow': ['rot_touv', 'rot_touv_body', 'selctsub', 'qerrcalc_body', 'jtlst_body', 'dtcb', 'rd', 'rand',
                    'leave', 'tcq', 'sv', 'sa']}
# Branches composed in C: the transfers among this file's own words (its
# labels and the TC calls of its subroutines, compiled as C calls).
INLINE = {'flow': {0o2103, 0o2107, 0o2144, 0o2146, 0o2147, 0o2164, 0o2200, 0o2246, 0o2252, 0o2256, 0o2262, 0o2264, 0o2265, 0o2266, 0o2305, 0o2307, 0o2321, 0o2323, 0o2345, 0o2350, 0o2354, 0o2374, 0o2431, 0o2441, 0o2446, 0o2447, 0o2451, 0o2466, 0o2475, 0o2501, 0o2504, 0o2515, 0o2527, 0o2540, 0o2551, 0o2566, 0o2574, 0o2603, 0o2607, 0o2612, 0o2615, 0o2626, 0o2661, 0o2675, 0o2715, 0o2727, 0o2743, 0o2753, 0o2754, 0o2777, 0o3005, 0o3010, 0o3022, 0o3031, 0o3032, 0o3051, 0o3057, 0o3060, 0o3063, 0o3114, 0o3146, 0o3173, 0o3236}, 'qr_data': {0o2566}, 'jtlst_body': {0o3131, 0o3142}, 'rot_touv_body': {0o3163, 0o3171}, 'selctsub': {0o3203, 0o3204, 0o3205}}
CHANNELS = {0o4: 'chan4', 0o5: 'chan5', 0o13: 'chan13', 0o31: 'chan31'}
SITES = {'lm_interbank_sites': 'src/lm_interbank_sites.inc', 'lm_q_r_axis_sites': 'src/lm_q_r_axis_sites.inc'}


def main():
    result, owned = check('src/lm_q_r_axis.c', {'Q_R-AXIS_RCS_AUTOPILOT.agc': None}, (0o17, 3), OWNERS, HELPERS,
                          INLINE, CHANNELS, SITES, work_name='q-r-axis-source')
    result['note'] = ('Fresh yaYUL assembly of the file reproduces the certified rope; every assembled word of '
                      'Q_R-AXIS_RCS_AUTOPILOT (bank 17 and MAKERUPT in fixed-fixed) has one native owner holding '
                      'its erasable address (the indexed base for INDEX orders), the rope value of its fixed '
                      'operand, its branch target (or composes it inline), its channel or its constant; the '
                      'IBNKCALL CADRs and the ALARM codes are compiled site operands, and the words no code '
                      'reaches are lm_q_r_axis_data.')
    (R / 'build/lm-q-r-axis-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
