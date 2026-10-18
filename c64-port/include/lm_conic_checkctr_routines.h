/* SPDX-License-Identifier: MIT */
#ifndef LM_CONIC_CHECKCTR_ROUTINES_H
#define LM_CONIC_CHECKCTR_ROUTINES_H
#include "agc_transfer.h"
/* The flight store's table for Luminary099 CONIC_SUBROUTINES' CHECKCTR
 * (12,2644), the RTB target of KEPLOOP's BRNCHCTR (RTB BHIZ) and LAMBERT's
 * BPL RTB (docs/FLIGHT-STORE.md). On the host, CHECKCTR runs only inside
 * lm_conic_compiled's routines (its native range), so no host run reaches
 * 12,2644 as a basic point. On the C64 the interpreter module runs those
 * orders, and its RTB leaves Z at CHECKCTR under FB 12. The body is
 * lm_conic_checkctr: it leaves Z at DANZIG, where the interpreter takes the
 * pending BHIZ (or the BPL's +0 EDOP) on. */
extern const agc_routine_table lm_conic_checkctr_routines;
#endif
