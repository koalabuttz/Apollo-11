/* SPDX-License-Identifier: MIT */
#ifndef LM_ALARM_CASES_H
#define LM_ALARM_CASES_H
#include "lm_alarm.h"
/* Each case enters one ALARM_AND_ABORT point and runs through the
 * trampoline (with lm_interbank_routines for BANKCALL, POSTJUMP and
 * SUPERSW) until a point neither owns: the caller's return, PRIODSPR,
 * ENEMA, V37KLEAN, MR.KLEAN or SERVIDLE, or SWRETURN after a BANKCALL'd
 * FLAGS returns (its TC Q leaves Q = 3, which lm_interbank_routines'
 * SWRETURN does not take; the code after it is entered directly instead). Case n uses kind n % kinds: every
 * genuine TC ALARM, ALARM2, BAILOUT, POODOO, ABORT2, BAILOUT1, POODOO1,
 * ALARM1 and ALMNCADR site (Q at its alarm code, FB of its bank), then
 * CCSHOLE, CURTAINS, VARALARM, WHIMPER, GOPOODOO, PRIOLARM and FLAGS as
 * their callers enter them (an alarm code is positive, as every caller's is), and the points control comes back to (MULTEXIT
 * targets, BANKCALL/SUPERSW returns, ALMCADR), and ALMNCADR (no TC site) with
 * Q at a random site's alarm code. Images, FB/EB, SUPERBNK, the
 * interrupt-enable and in-interrupt states are pseudo-random; each FAILREG
 * word is +0, -0, positive or negative a quarter of the time.
 * tools/lm_alarm_reference.c does the same in the original program. */
#define LM_AL_CASES 4096u
void lm_al_input(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n);
enum exec_status lm_al_run(apollo_overlay_state *,p66_bias_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/lm_alarm_freeze.py). */
uint32_t lm_al_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
uint16_t lm_al_kinds(void);
#endif
