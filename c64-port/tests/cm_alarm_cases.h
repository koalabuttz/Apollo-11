/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_CASES_H
#define CM_ALARM_CASES_H
#include "cm_alarm.h"
/* Each case enters one ALARM_AND_ABORT point and runs through the
 * trampoline (with cm_interbank_routines for BANKCALL and POSTJUMP) until a
 * point neither owns: the caller's return, PRIODSPR, DOWNFLAG, VAC5STOR,
 * MR.KLEAN or ENEMA. Case n uses kind n % kinds: every genuine TC ALARM,
 * ALARM2, BAILOUT (= ABORT) and POODOO site (Q at its alarm code, FB of its
 * bank), then CCSHOLE, CURTAINS, VARALARM and PRIOLARM as their callers
 * enter them (an alarm code is positive, as every caller's is), and the
 * points control comes back to (MULTEXIT targets, BANKCALL and DOWNFLAG
 * returns, ALMCADR); the VAC5STOR returns of BAILOUT and POODOO with ALMCADR
 * at a random BAILOUT or POODOO site's alarm code. Images, FB/EB, SUPERBNK,
 * the interrupt-enable and in-interrupt states are pseudo-random; each
 * FAILREG word is +0, -0, positive or negative a quarter of the time.
 * tools/cm_alarm_reference.c does the same in the original program. */
#define CM_AL_CASES 4096u
void cm_al_input(apollo_overlay_state *,cm_native_io *,agc_hardware *,uint16_t n);
enum exec_status cm_al_run(apollo_overlay_state *,cm_native_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/cm_alarm_freeze.py). */
uint32_t cm_al_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
uint16_t cm_al_kinds(void);
#endif
