/* SPDX-License-Identifier: MIT */
#ifndef CM_V37_CASES_H
#define CM_V37_CASES_H
#include "cm_v37.h"
/* Each case enters one bank 04 / bank 20 entry (case n: entry n % 35) and
 * runs through the trampoline (with cm_fresh_start_routines for the bank 05
 * code it reaches, IBNKCALL V37KLEAN/P00KLEAN and POSTJUMP GOPROG2, and
 * cm_interbank_routines for BANKCALL, IBNKCALL and POSTJUMP) until a point
 * none owns, SWRETURN (VAC5STOR's return, or a BANKCALL'd KLEAN entry
 * returning with Q = 3) or ISWRETRN (an IBNKCALL'd KLEAN entry returning
 * with Q = 3); cm_interbank_complete takes neither with Q = 3. The image is
 * pseudo-random within the program's domain: V37's request (A) and
 * MMNUMBER a major mode PREMM1 lists, P00, P20 or any of 0-0177; MINDEX an
 * index of the 33-word tables; MODREG +0, P20 or a mode; the IMU-busy and
 * NODO bits mostly clear so V37 gets past CANTR00; INITSUB's return word
 * (MPAC +1) 2001 or 2146 where RAKE's TC MPAC +1 executes it; restart-group
 * phase pairs that agree; FB the entry's bank, EB, SUPERBNK, the channels
 * (5, 6, 11-14 without the bits yaAGC acts on as a peripheral, 15, 16,
 * 33), the trap latches and the interrupt-enable state pseudo-random.
 * tools/cm_v37_reference.c does the same in the original program. */
#define CM_V37_CASES 4096u
#define CM_V37_SWRETURN 04675u
#define CM_V37_SWRETURN_POINT (uint16_t)(2048u+(CM_V37_SWRETURN-04000u))
#define CM_V37_ISWRETRN 04753u
#define CM_V37_ISWRETRN_POINT (uint16_t)(2048u+(CM_V37_ISWRETRN-04000u))
void cm_v37_input(apollo_overlay_state *,cm_fresh_io *,uint16_t n);
enum exec_status cm_v37_run(apollo_overlay_state *,cm_fresh_io *,uint16_t n,uint16_t *steps);
/* Digest in the oracle's order (tools/cm_v37_freeze.py). */
uint32_t cm_v37_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
#endif
