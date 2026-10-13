/* SPDX-License-Identifier: MIT */
#ifndef LM_SVCT3_H
#define LM_SVCT3_H
#include "p66_bias.h"
enum lm_svct3_exit { LM_SVCT3_TASKOVER=0,LM_SVCT3_NOVAC=1,LM_SVCT3_FIXDELAY=2,LM_SVCT3_INVALID=3 };
/* Luminary099 WAITLIST.agc's end task, entered as the Waitlist dispatches
 * ENDTASK's -2CADR (BB 04003): 0=SVCT3 05240, 1=its NOVAC return 05255,
 * 2=SVCT3X's TC SVCT3 at 05260. CCS FLAGWRD2: a positive or +0 drift flag
 * ends at TASKOVER 05261; otherwise CKIMUSE's CCS IMUCADR: +0 calls NOVAC
 * 05072 (A=PRIO35, Q=05253 at the NBDONLY 2CADR), anything else FIXDELAY
 * 05221 (Q=05257 at DEC 500). Each phase stops where the source calls a
 * service; a phase or bank outside this is refused with the image unchanged. */
enum lm_svct3_exit lm_svct3_phase(apollo_overlay_state *,p66_bias_io *,uint8_t);
#endif
