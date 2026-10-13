/* SPDX-License-Identifier: MIT */
#ifndef LM_V37_CASES_H
#define LM_V37_CASES_H
#include "lm_v37.h"
#include "lm_fresh_start.h"
/* Each case enters one bank 04 entry (case n: entry n % 32) and runs through
 * the trampoline, with lm_fresh_start_routines (V37KLEAN, P00KLEAN, GOPROG2
 * and what they reach in bank 05) and lm_interbank_routines (BANKCALL,
 * IBNKCALL, POSTJUMP), until a point none owns, or SWRETURN, or ISWRETRN
 * (a KLEAN entry under IBNKCALL returns there with Q = 3, which
 * lm_interbank does not take). The image is
 * pseudo-random within the program's domain: the requested major mode (A at
 * V37, MMNUMBER) a mode in PREMM1, P00, P70, P71 or any of 0-0177, at V37
 * mostly with IMODES30 bit 6 and NODOFLAG clear; MINDEX
 * 0-29; MODREG P20, P22, P25, a neighbour, P00, a mode as above or random
 * (P20 in a third of the P20 or P25 requests); the flag words,
 * IMODES30, FB/EB, SUPERBNK, the channels and restart groups as in
 * tests/lm_fresh_start_cases.c. tools/lm_v37_reference.c does the same in
 * the original program. */
#define LM_V37_CASES 4096u
#define LM_V37_SWRETURN_POINT (uint16_t)(2048u+(04631u-04000u))
#define LM_V37_ISWRETRN_POINT (uint16_t)(2048u+(04707u-04000u))
void lm_v37_input(apollo_overlay_state *,lm_fresh_io *,agc_hardware *,uint16_t n);
enum exec_status lm_v37_run(apollo_overlay_state *,lm_fresh_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/lm_v37_freeze.py). */
uint32_t lm_v37_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
