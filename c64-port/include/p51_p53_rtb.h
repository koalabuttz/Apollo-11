/* SPDX-License-Identifier: MIT */
#ifndef P51_P53_RTB_H
#define P51_P53_RTB_H
#include "overlay_runtime.h"
#include "p66_bias.h"
/* P51-P53's basic routines that the file's own RTBs run, as the original
 * does from the RTB's transfer (Z at the routine, FB its bank):
 * RDCDUS (Luminary 14,3413, Comanche 15,2324) reads CDUX/Y/Z into the
 * job's VAC words 1-3 with interrupts inhibited, then TC DANZIG; Comanche
 * SET1/PDT (14,3275) copies TIME1 into 1/PIPADT, then TCF DANZIG. Each
 * returns RTB_OK with Z at DANZIG (Q the word after a TC), or
 * RTB_OUT_OF_SCOPE when Z, FB or the image is not one the routine runs on. */
enum p51_p53_rtb_kind {P51_LM_RDCDUS,P51_CM_RDCDUS,P51_CM_SET1PDT};
uint8_t p51_p53_rtb(apollo_overlay_state *, p66_bias_io *, enum p51_p53_rtb_kind);
#endif
