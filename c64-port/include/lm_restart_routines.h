/* SPDX-License-Identifier: MIT */
#ifndef LM_RESTART_ROUTINES_H
#define LM_RESTART_ROUTINES_H
#include "agc_transfer.h"
#include "lm_restart_select.h"
/* Luminary099 RESTARTS_ROUTINE as native routines of the control-transfer
 * ABI (docs/NATIVE_TRANSFER_ABI.md): RESTARTS 01,3526, GETPART2 01,3541 and
 * PHSPART2 01,3763 select a restart and leave Z at TC GOLOC-1 (or the X.1
 * display's TC FINDVAC); the display return 01,3550 (TC RTRNCADR, TC
 * SWRETURN); and the two erasable words RESTARTS stores for later execution,
 * GOLOC-1 (TC FINDVAC/NOVAC/WAITLIST/LONGCALL) and GOLOC+2 (TC SWRETURN,
 * PHSPART2 or GETPART2), registered at their erasable keys. The selected
 * service receives the restart's 2CADR at Q (GOLOC, or the INITDSP site in
 * lm_restart_operands) and returns to Q+2. Any other word stored there is
 * EXEC_OUT_OF_SCOPE: no original producer stores it. */
extern const agc_routine_table lm_restart_routines;
/* The inline 2CADR INITDSP after RESTARTS' TC FINDVAC (01,3546-3547). */
extern const agc_site_operand lm_restart_operands[2];
#endif
