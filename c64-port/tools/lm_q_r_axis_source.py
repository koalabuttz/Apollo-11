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
OWNERS = [(B, 0o2072, 0o2106, 'callqerr_part'), (B, 0o2107, 0o2170, 'rcs'),
          (B, 0o2171, 0o2171, 'lm_q_r_axis_sites'), (B, 0o2172, 0o2177, 'alarmret_body'),
          (B, 0o2200, 0o2212, 'qrcontrl'), (B, 0o2213, 0o2213, 'lm_interbank_sites'),
          (B, 0o2214, 0o2272, 'minret_part'), (B, 0o2273, 0o2316, 'minrtn'), (B, 0o2317, 0o2320, 'minret_part'),
          (B, 0o2321, 0o2322, 'rcs'), (B, 0o2323, 0o2342, 'chekstik'), (B, 0o2343, 0o2343, 'qr_data'),
          (B, 0o2344, 0o2452, 'chekstik'), (B, 0o2453, 0o2453, 'lm_interbank_sites'),
          (B, 0o2454, 0o2531, 'enteruvret_part'), (B, 0o2532, 0o2570, 'backhand'), (B, 0o2571, 0o2572, 'qr_data'),
          (B, 0o2573, 0o2573, 'enteruvret_part'), (B, 0o2574, 0o2606, 'trygts'), (B, 0o2607, 0o2611, 'gotogts'),
          (B, 0o2612, 0o2616, 'trygts'), (B, 0o2617, 0o2617, 'lm_interbank_sites'), (B, 0o2620, 0o2623, 'timegmblret'),
          (B, 0o2624, 0o2625, 'gotogts'), (B, 0o2626, 0o2660, 'qerrcalc_body'), (B, 0o2661, 0o2722, 'stillrcs'),
          (B, 0o2723, 0o2723, 'lm_interbank_sites'), (B, 0o2724, 0o2726, 'spsret_body'), (B, 0o2727, 0o2774, 'aftertj'),
          (B, 0o2775, 0o2776, 'writepret'), (B, 0o2777, 0o3003, 'norotat'), (B, 0o3004, 0o3004, 'norotatret'),
          (B, 0o3005, 0o3007, 'looper'), (B, 0o3010, 0o3041, 'doskip'), (B, 0o3042, 0o3050, 'notminret_part'),
          (B, 0o3051, 0o3062, 'feedback'), (B, 0o3063, 0o3073, 'xtrans'), (B, 0o3074, 0o3102, 'writeuret_part'),
          (B, 0o3103, 0o3104, 'writevret_part'), (B, 0o3105, 0o3110, 'aftertj'), (B, 0o3111, 0o3112, 'qr_data'),
          (B, 0o3113, 0o3113, 'stillrcs'), (B, 0o3114, 0o3145, 'jtlst_body'), (B, 0o3146, 0o3172, 'rot_touv_body'),
          (B, 0o3173, 0o3222, 'selctsub'), (B, 0o3223, 0o3223, 'lm_q_r_axis_sites'), (B, 0o3224, 0o3224, 'selalarmret'),
          (B, 0o3225, 0o3235, 'selctsub'), (B, 0o3236, 0o3251, 'closeout_part'), (3, 0o7754, 0o7755, 'closeout_part')]
# CALLQERR's step machine runs as parts (each hands Z on to the next, or calls
# a shared tail); together they hold what the single step machine held.
PARTS = ['callqerr_part', 'rcs', 'alarmret_body', 'qrcontrl', 'minret_part', 'minrtn', 'chekstik', 'enteruvret_part',
         'backhand', 'trygts', 'gotogts', 'timegmblret', 'stillrcs', 'spsret_body', 'aftertj', 'writepret', 'norotat',
         'norotatret', 'looper', 'doskip', 'notminret_part', 'feedback', 'xtrans', 'writeuret_part', 'writevret_part',
         'selalarmret', 'closeout_part']
FLOW = ['handoff', 'rot_touv', 'rot_touv_body', 'selctsub', 'qerrcalc_body', 'jtlst_body', 'dtcb', 'rd', 'rand',
        'leave', 'tcq', 'sv', 'sa'] + PARTS
HELPERS = {'qerrcalc_body': ['msu', 'tcq'], 'rot_touv_body': ['tcq'], 'selctsub': ['tcq', 'sv'],
           'jtlst_body': ['wor13', 'tcq', 'rot_touv_body'],
           **{f: FLOW for f in PARTS}}
# Branches composed in C: the transfers among this file's own words (its
# labels and the TC calls of its subroutines, compiled as C calls).
FLOW_INLINE = {0o2103, 0o2107, 0o2144, 0o2146, 0o2147, 0o2164, 0o2200, 0o2246, 0o2252, 0o2256, 0o2262, 0o2264,
               0o2265, 0o2266, 0o2305, 0o2307, 0o2321, 0o2323, 0o2345, 0o2350, 0o2354, 0o2374, 0o2431, 0o2441,
               0o2446, 0o2447, 0o2451, 0o2466, 0o2475, 0o2501, 0o2504, 0o2515, 0o2527, 0o2540, 0o2551, 0o2566,
               0o2574, 0o2603, 0o2607, 0o2612, 0o2615, 0o2626, 0o2661, 0o2675, 0o2715, 0o2727, 0o2743, 0o2753,
               0o2754, 0o2777, 0o3005, 0o3010, 0o3022, 0o3031, 0o3032, 0o3051, 0o3057, 0o3060, 0o3063, 0o3114,
               0o3146, 0o3173, 0o3236}
INLINE = {**{f: FLOW_INLINE for f in PARTS}, 'qr_data': {0o2566}, 'jtlst_body': {0o3131, 0o3142},
          'rot_touv_body': {0o3163, 0o3171}, 'selctsub': {0o3203, 0o3204, 0o3205}}
CHANNELS = {0o4: 'channel4', 0o5: 'channel5', 0o13: 'channel13', 0o31: 'channel31'}
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
