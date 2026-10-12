/* SPDX-License-Identifier: MIT */
#ifndef LM_FRESH_START_CASES_H
#define LM_FRESH_START_CASES_H
#include "lm_fresh_start.h"
/* Each case enters one bank 05 / GOTOPOOH entry (case n: entry n % 11) and
 * runs through the trampoline (with lm_interbank_routines for POSTJUMP and
 * SWCALL) until a point neither owns, or SWRETURN (a BANKCALL'd KLEAN entry
 * returning with Q = 3, which lm_interbank does not take). The image is
 * pseudo-random within the program's domain: restart-group phase pairs that
 * agree (one pair corrupted in some cases: a flipped bit, or -PHASEn equal
 * to PHASEn), MPAC +5/+6 as the group scan
 * leaves them for the PINACT return, ERESTORE +0, -0, the address ERASCHK
 * was testing (061-01776, SKEEP7 equal to it) or a corrupt value, KLEAN
 * callers' Q a fixed-fixed return; FB/EB, SUPERBNK, the input channels (15,
 * 16 with the ELR key sometimes held, 33), the output channels (5, 6, 11,
 * 12, 13 and 14 without the bits yaAGC acts on as a peripheral), the trap
 * latches and the interrupt-enable state pseudo-random.
 * tools/lm_fresh_start_reference.c does the same in the original program. */
#define LM_FS_CASES 4096u
#define LM_FS_SWRETURN_POINT (uint16_t)(2048u+(04631u-04000u))
void lm_fs_input(apollo_overlay_state *,lm_fresh_io *,agc_hardware *,uint16_t n);
enum exec_status lm_fs_run(apollo_overlay_state *,lm_fresh_io *,agc_hardware *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/lm_fresh_start_freeze.py). */
uint32_t lm_fs_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
