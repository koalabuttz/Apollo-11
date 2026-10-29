#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for the native Luminary099 P-AXIS_RCS_AUTOPILOT
(src/lm_p_axis.c), checked by tools/lm_basic_source.py. Writes
build/lm-p-axis-source-map.json."""
import json, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from lm_basic_source import check  # noqa: E402

OWNERS = [(0o2213, 0o2222, 'paxis'), (0o2223, 0o2223, 'chekbits2'), (0o2224, 0o2255, 'afterchek_body'),
          (0o2256, 0o2262, 'onetotwos'), (0o2263, 0o2307, 'subdivde'), (0o2310, 0o2314, 'oversub'),
          (0o2315, 0o2315, 'subdivde'), (0o2316, 0o2450, 'rateloop'), (0o2451, 0o2543, 'traps'),
          (0o2544, 0o2601, 'qrate'), (0o2602, 0o2622, 'rrate'), (0o2623, 0o2623, 'pa_data'),
          (0o2624, 0o2646, 'rrate'), (0o2647, 0o2652, 'afterchek_body'), (0o2653, 0o2665, 'afteracdt_body'),
          (0o2666, 0o2666, 'superjob'), (0o2667, 0o2737, 'pretimck'), (0o2740, 0o2771, 'chkvisfz'), (0o2772, 0o2772, 'lm_p_axis_sites'),
          (0o2773, 0o3001, 'abortyz2'), (0o3002, 0o3012, 'chkvisfz'), (0o3013, 0o3070, 'manual'), (0o3071, 0o3131, 'detentck'),
          (0o3132, 0o3132, 'pa_data'), (0o3133, 0o3133, 'raterror'), (0o3134, 0o3134, 'detentck'),
          (0o3135, 0o3135, 'raterror'), (0o3136, 0o3152, 'detentck'),
          (0o3153, 0o3153, 'lm_interbank_sites'), (0o3154, 0o3204, 'detentck'), (0o3205, 0o3217, 'zeroenbl'),
          (0o3220, 0o3226, 'detentck'), (0o3227, 0o3347, 'raterror'), (0o3350, 0o3402, 'jetselect'), (0o3403, 0o3422, 'writepret_body'),
          (0o3423, 0o3423, 'lm_interbank_sites'), (0o3424, 0o3430, 'jtlstret_body'),
          (0o3431, 0o3440, 'altsyst'), (0o3441, 0o3441, 'jetselect'), (0o3442, 0o3442, 'jetsoff'),
          (0o3443, 0o3445, 'jetsoffret_body'), (0o3446, 0o3462, 'calcperr_body'),
          (0o3463, 0o3476, 'urgency'), (0o3477, 0o3477, 'lm_interbank_sites'), (0o3500, 0o3504, 'jetselect'),
          (0o3505, 0o3510, 'urgency'), (0o3511, 0o3511, 'lm_interbank_sites'), (0o3512, 0o3530, 'jetselect'),
          (0o3531, 0o3546, 'selectp'), (0o3547, 0o3547, 'lm_p_axis_sites'), (0o3550, 0o3550, 'jetsoff'),
          (0o3551, 0o3571, 'selectp'), (0o3572, 0o3611, 'chkvisfz'), (0o3612, 0o3612, 'writepret_body'),
          (0o3613, 0o3613, 'chkvisfz'), (0o3614, 0o3615, 'pa_data'), (0o3616, 0o3616, 'rrate'),
          (0o3617, 0o3617, 'zeroenbl'), (0o3620, 0o3620, 'rateloop'), (0o3621, 0o3622, 'writepret_body'),
          (0o3623, 0o3623, 'pa_data'), (0o3624, 0o3640, 'qraxis')]
# The rate loop's parts (each hands Z on to the next) share the old
# single step machine's helpers.
PARTS = ['rateloop', 'traps', 'qrate', 'rrate', 'pretimck', 'chkvisfz', 'manual', 'detentck', 'raterror', 'jetselect', 'urgency',
         'writepret_body', 'jtlstret_body', 'jetsoffret_body', 'abortyz2', 'jetsoff', 'qraxis']
FLOW = ['handoff', 'abortyz2', 'jetsoff', 'qraxis', 'subdivde', 'oversub', 'zeroenbl', 'altsyst', 'calcperr_body',
        'selectp', 'msu', 'dtcb', 'rd', 'rand', 'wand', 'rxor_l', 'leave', 'sv', 'sa']
HELPERS = {'paxis': ['afterchek_body', 'leave'], 'chekbits2': ['leave'],
           'afterchek_body': ['afteracdt_body', 'onetotwos', 'msu', 'leave'], 'onetotwos': ['tcq'],
           'subdivde': ['tcq'], 'oversub': ['tcq'], 'zeroenbl': ['wor13', 'tcq'], 'altsyst': ['rxor_l', 'tcq'],
           'calcperr_body': ['msu', 'tcq'], 'selectp': ['tcq', 'sv'],
           **{f: FLOW for f in PARTS}}
# Branches composed in C: the transfers among this file's own words (its
# labels and the TC calls of its subroutines, compiled as C calls).
FLOW_INLINE = {0o2263, 0o2266, 0o2310, 0o2462, 0o2466, 0o2472, 0o2507, 0o2517, 0o2544, 0o2555, 0o2602, 0o2624,
               0o2667, 0o2672, 0o2675, 0o2710, 0o2712, 0o2714, 0o2724, 0o2734, 0o2740, 0o2753, 0o3002, 0o3013,
               0o3014, 0o3027, 0o3054, 0o3062, 0o3064, 0o3065, 0o3071, 0o3120, 0o3125, 0o3136, 0o3152, 0o3154,
               0o3156, 0o3205, 0o3220, 0o3223, 0o3227, 0o3235, 0o3261, 0o3267, 0o3274, 0o3300, 0o3312, 0o3325,
               0o3330, 0o3346, 0o3350, 0o3355, 0o3365, 0o3366, 0o3377, 0o3416, 0o3431, 0o3441, 0o3442, 0o3446,
               0o3463, 0o3467, 0o3505, 0o3522, 0o3531, 0o3551, 0o3624}
INLINE = {'afterchek_body': {0o2256, 0o2647},
          'subdivde': {0o2274, 0o2304},
          'selectp': {0o2773, 0o3532, 0o3544, 0o3551},
          **{f: FLOW_INLINE for f in PARTS}}
CHANNELS = {0o1: 'rxor_l', 0o12: 'channel12', 0o13: 'channel13', 0o31: 'channel31'}
SITES = {'lm_interbank_sites': 'src/lm_interbank_sites.inc', 'lm_p_axis_sites': 'src/lm_p_axis_sites.inc'}


def main():
    result, owned = check('src/lm_p_axis.c', {'P-AXIS_RCS_AUTOPILOT.agc': None}, 0o16, OWNERS, HELPERS, INLINE,
                          CHANNELS, SITES, work_name='p-axis-source')
    result['note'] = ('Fresh yaYUL assembly of the file reproduces the certified rope; every assembled word of '
                      'P-AXIS_RCS_AUTOPILOT has one native owner holding its erasable address (the indexed base for '
                      'INDEX orders), the rope value of its fixed operand, its branch target (or composes it '
                      'inline), its channel or its constant; the IBNKCALL CADRs and the ALARM codes are compiled '
                      'site operands, and the data words no code here reads are lm_p_axis_data.')
    (R / 'build/lm-p-axis-source-map.json').write_text(json.dumps(result, indent=1) + '\n')
    print('PASS', result['source_words'], 'assembled words on', result['source_lines'], 'source lines map to',
          len(owned), 'native owners', flush=True)


if __name__ == '__main__':
    main()
