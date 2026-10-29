/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_BLANK_H
#define DSKY_NV_BLANK_H
#include "dsky_nv_admission.h"
/* Original LM/CM BLANKDSP 41,3516 through ENTSET-2 entry 3566.
 * Input A=abs(NVTEMP)-1, NVTEMP=-1..-4; coherent bank41, SUPER100,
 * ordinary erasable words and legal external channel/IRQ. Canonical image
 * authoritative; caller excludes producers. Includes FLASHOFF, source CCS
 * signed-zero relay behavior and overflow-corrected NOUT increment.
 * Selected ENTSET-2 body is separate. Unsupported preserves all objects. */
enum p66_bias_status dsky_nv_blank(apollo_overlay_state *,p66_bias_io *,agc_word *,enum dsky_nv_mission);
#endif
