/* SPDX-License-Identifier: MIT */
#ifndef LM_DOWNTELEMETRY_CASES_H
#define LM_DOWNTELEMETRY_CASES_H
#include "lm_downtelemetry.h"
/* Each case starts as a producer leaves the downlink: DNTMGOTO = DNPHASE1
 * (STARTSUB's LDNPHAS1) with DNLSTCOD one of the six downlists, or DNDUMPI
 * (DNEDUMP's LDNDUMPI) with DUMPCNT asking for 1, 2 or 4 erasable dumps.
 * The rest of the image is pseudo-random. It then takes a sequence of
 * DOWNRUPTs: before each, the DOWNRUPT lead-in state (A = an interrupted BB,
 * Q pseudo-random, FB 05/EB 0, Z = DODOWNTM) and channels 11-14 and 30-33
 * are set from the case's seed (channel 14 without the IMU CDU, optics and
 * gyro drive bits, which yaAGC acts on as a peripheral); the downrupt runs through the trampoline to
 * TCF RESUME. Ordinary cases take enough downrupts to pass the end of their
 * downlist and start it again; dump cases run their dumps to completion and
 * return to the downlist. tools/lm_downtelemetry_reference.c does the same
 * in the original program. */
#define LM_DNTM_CASES 48u
#define LM_DNTM_DUMP_CASES 3u   /* the last three */
uint16_t lm_dntm_steps(uint16_t n);
void lm_dntm_begin(apollo_overlay_state *,lm_dntm_io *,uint16_t n);
void lm_dntm_entry(apollo_overlay_state *,lm_dntm_io *,uint16_t n,uint16_t step);
/* One downrupt from the entry state to RESUME; EXEC_OK when it got there. */
enum exec_status lm_dntm_downrupt(apollo_overlay_state *,lm_dntm_io *);
/* Digest of a whole case, in the oracle's order (tools/lm_downtelemetry_freeze.py). */
uint32_t lm_dntm_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
