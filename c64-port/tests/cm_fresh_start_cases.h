/* SPDX-License-Identifier: MIT */
#ifndef CM_FRESH_START_CASES_H
#define CM_FRESH_START_CASES_H
#include "cm_fresh_start.h"
/* Each case enters one bank 05 / GOTOPOOH entry (case n: entry n % 17) and
 * runs through the trampoline (with cm_interbank_routines for BANKCALL,
 * IBNKCALL, POSTJUMP and SWCALL) until a point neither owns, or SWRETURN (a
 * BANKCALL'd KLEAN entry returning with Q = 3, which cm_interbank_complete
 * does not take). The
 * image is pseudo-random within the program's domain: restart-group phase
 * pairs that agree (one pair corrupted in some cases), MPAC +5/+6 as the
 * group scan leaves them for the PINACT return, ERESTORE +0, -0, the
 * address ERASCHK was testing (061-01776, SKEEP7 equal to it) or a corrupt
 * value, a KLEAN caller's Q SWRETURN (BUF2 a bank 04 return) or another
 * fixed-fixed word; FB/EB, SUPERBNK, the input channels (15, 16 with the
 * ELR key sometimes held, 33; in a quarter of the cases the AGC warning
 * persisting, channel 33 bit 14 0 and set again after each reset), the output channels (5, 6, 11, 12, 13 and
 * 14 without the bits yaAGC acts on as a peripheral), the trap latches and
 * the interrupt-enable state pseudo-random.
 * tools/cm_fresh_start_reference.c does the same in the original program. */
#define CM_FS_CASES 4096u
#define CM_FS_SWRETURN_POINT (uint16_t)(2048u+(04675u-04000u))
void cm_fs_input(apollo_overlay_state *,cm_fresh_io *,uint16_t n);
enum exec_status cm_fs_run(apollo_overlay_state *,cm_fresh_io *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/cm_fresh_start_freeze.py). */
uint32_t cm_fs_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
