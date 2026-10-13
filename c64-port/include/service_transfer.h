/* SPDX-License-Identifier: MIT */
#ifndef SERVICE_TRANSFER_H
#define SERVICE_TRANSFER_H
/* SERVICE_ROUTINES entries of both missions as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md, docs/SERVICE-TRANSFER.md).
 * Each routine performs service_routines_step for its entry at its original
 * address and leaves Z at the continuation (the caller's return, JOBSLEEP,
 * JOBWAKE, TASKOVER, ENDOFJOB, DLY2 or the GENTRAN back edge), where the
 * trampoline enters that point's own routine. Inline operands after a fixed
 * `TC UPFLAG`, `TC DOWNFLAG` or `TC GENTRAN` come from the program's compiled
 * operand table (lm_service_sites, cm_service_sites); GENTRAN's fixed source
 * words from the program's passive constant reader. LM io is a p66_bias_io,
 * CM io a cm_native_io. */
#include "agc_transfer.h"
extern const agc_routine_table lm_service_routines, cm_service_routines;
#if defined(CM_SERVICE_DELAYJOB_ONLY)
extern const agc_routine_table cm_service_delayjob_routines; /* CM DELAYJOB alone */
#endif
const agc_site_operand *lm_service_site_table(uint16_t *count);
const agc_site_operand *cm_service_site_table(uint16_t *count);
#endif
