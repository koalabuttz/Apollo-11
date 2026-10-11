/* SPDX-License-Identifier: MIT */
#ifndef DSKY_FIXED_DISPLAY_H
#define DSKY_FIXED_DISPLAY_H
#include "dsky_nv_admission.h"
#include <stdbool.h>
enum dsky_fixed_display_stage {DSKY_FIXED_ENTER,DSKY_FIXED_DATA,DSKY_FIXED_FORMAT};
enum dsky_fixed_display_route {DSKY_FIXED_DATA_ENTRY,DSKY_FIXED_READ_READY,DSKY_FIXED_END,DSKY_FIXED_CCS_HOLE};
/* Actual DSPFMEM41,3353 lead-in ->SUPDACAL, copied passive word read ->3361, then original octal
 * formatter and ENDSPF -> real ENDOFJOB or CCSHOLE. Coherent bank41/allEB,
 * selecting SUPER, IRQ0/1, ordinary RAM/rawA/L. Producer exclusion and
 * distinct objects; passive page read occurs after ENTER returns; no data pointer survives reload.
 * Unsupported inputs preserve image/IO/route. No decoder.
 * The caller owns actual END/alarm bodies and device relay drain. */
enum p66_bias_status dsky_fixed_display(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_fixed_display_stage,agc_word copied_passive_word,enum dsky_fixed_display_route *);
/* Nonmutating address at actual SUPDACAL entry. Read immutable data after
 * control frame returns, then pass that copied word to DATA. */
bool dsky_fixed_display_address(const apollo_overlay_state *,const p66_bias_io *,enum dsky_nv_mission,uint16_t *);
/* Default passive all-original data reader; link separately from arithmetic. */
bool dsky_fixed_display_data(enum dsky_nv_mission,uint16_t,agc_word *,const void *);
#endif
