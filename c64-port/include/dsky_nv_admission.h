/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_ADMISSION_H
#define DSKY_NV_ADMISSION_H
#include "p66_bias.h"
#include "agc_word.h"
enum dsky_nv_mission {DSKY_NV_LM,DSKY_NV_CM};
enum dsky_nv_entry {DSKY_NV_NVSUB,DSKY_NV_NVMONOPT,DSKY_NV_NVSUB1,DSKY_NV_NVSUB2};
enum dsky_nv_route {DSKY_NV_BLOCKED,DSKY_NV_VERB_ONLY,DSKY_NV_NOUN_ONLY,DSKY_NV_VERB_NOUN,DSKY_NV_BLANK,DSKY_NV_ALARM};
/* Original fixed-fixed NVSUB/NVMONOPT and internal NVSUB1/2 bodies through
 * actual source-selected callee entry (no formatter/blank/alarm execution).
 * Canonical image/IO; fixed callerQ02000..07777, DSPLOCK0..7. Available calls
 * take positive packed7-bit VN, eitherzero or sourceblank -1..-4. Blocked
 * calls accept any sign-agreed A. NVSUB1 accepts those same available inputs;
 * NVSUB2 accepts generated noun/verb0..127. Ordinary MPAC/flag words and
 * coherent banks/REGZERO0 required; caller excludes producers. Unsupported
 * contexts preserve image/IO/route. No instruction words are decoded. */
enum p66_bias_status dsky_nv_admit(apollo_overlay_state *,p66_bias_io *,
 enum dsky_nv_mission,enum dsky_nv_entry,enum dsky_nv_route *);
#endif
