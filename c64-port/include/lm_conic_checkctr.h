/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_CHECKCTR_H
#define LM_CONIC_CHECKCTR_H
#include "overlay_runtime.h"
/* Luminary099 CONIC_SUBROUTINES' CHECKCTR (12,2644), which BRNCHCTR and
 * LAMBERT reach by RTB: ITERCTR (FIXLOC +22D) -1 into ITERCTR and MPAC,
 * then TC DANZIG. RTB_OK with Z at DANZIG and Q the word after the TC, or
 * RTB_OUT_OF_SCOPE (nothing changed) when Z is not at CHECKCTR under FB 12
 * or the count would overflow (its TS would skip the TC DANZIG). */
uint8_t lm_conic_checkctr(apollo_overlay_state *);
#endif
