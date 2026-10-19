/* SPDX-License-Identifier: MIT */
#ifndef P51_P53_RTB_ROUTINES_H
#define P51_P53_RTB_ROUTINES_H
#include "agc_transfer.h"
/* The flight store's tables for P51-P53's RTB targets (docs/FLIGHT-STORE.md):
 * Luminary099 RDCDUS (14,3413), Comanche055 RDCDUS (15,2324) and SET1/PDT
 * (14,3275). On the host they run only inside the p51-p53 compiled
 * routines (native_ranges), so no host run reaches them as basic points. On
 * the C64 the interpreter module runs the file's interpretive orders, and
 * its RTB leaves Z at the routine under FB of its bank. The body is
 * p51_p53_rtb: it leaves Z at DANZIG, where the interpreter goes on. */
extern const agc_routine_table lm_p51_p53_rtb_routines;
extern const agc_routine_table cm_p51_p53_rtb_routines;
#endif
