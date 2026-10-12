/* SPDX-License-Identifier: MIT */
#ifndef LM_DAP_INTERFACE_H
#define LM_DAP_INTERFACE_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 DAP_INTERFACE_SUBROUTINES (bank 20, 02123-02225) as native
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md); io is
 * p66_bias_io. Every label a caller reaches, and every point a call inside
 * the file returns to, is a routine at its physical address:
 *
 *   RESTORDB  20,2123   crew deadband from DAPBOOLS, then CALLACCS
 *   RESTORDB+1 20,2124  (ALLCOAST's TC) the same from A
 *   SETMAXDB  20,2127   5 degree deadband, CALLACCS
 *   SETMINDB  20,2140   0.3 degree deadband, CALLACCS
 *   CALLACCS+4 20,2137  return from NOVAC: TC RUPTREG1
 *   PFLITEDB  20,2142   TC ZATTEROR; return 20,2145: 1 degree, CALLACCS
 *   ZATTEROR  20,2153   CDU desired = CDU, then the STOPRATE clears
 *   STOPRATE  20,2165   nine rate/error clears, TC Q
 *   ALLCOAST  20,2204   TC STOPRATE; return 20,2207: AOS clears, DRIFTBIT,
 *                       TC RESTORDB+1; return 20,2225: TC RUPTREG2
 *
 * CALLACCS (CAF PRIO27; TC NOVAC; 2CADR 1/ACCJOB) ends at NOVAC with Q at
 * the 2CADR, so the job is admitted by the native NOVAC (lm_exec_routines).
 * The returns TC Q, TC RUPTREG1 and TC RUPTREG2 execute the stored word as a
 * TC; a word that is not a TC into fixed memory (02000-07777) has
 * no producer and is EXEC_OUT_OF_SCOPE with the image unchanged, as is a
 * TS at 20,2210.. with A overflowed (no producer: A is +0 there). */
extern const agc_routine_table lm_dap_interface_routines;

#define LM_DAP_RESTORDB 02123u
#define LM_DAP_SETMAXDB 02127u
#define LM_DAP_ACCRETURN 02137u
#define LM_DAP_SETMINDB 02140u
#define LM_DAP_PFLITEDB 02142u
#define LM_DAP_ZATTEROR 02153u
#define LM_DAP_STOPRATE 02165u
#define LM_DAP_ALLCOAST 02204u
#define LM_DAP_FB 040000u     /* bank 20 */
#define LM_DAP_FIRST 02123u   /* first and last word of the file */
#define LM_DAP_LAST 02225u
/* Erasable (EBANK6 is EB 03000). */
#define LM_DAP_DAPBOOLS 0111u
#define LM_DAP_DB 01346u
#define LM_DAP_RUPTREG1 070u
#define LM_DAP_RUPTREG2 071u
#endif
