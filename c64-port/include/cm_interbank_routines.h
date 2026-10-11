/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_ROUTINES_H
#define CM_INTERBANK_ROUTINES_H
/* Comanche055 INTER-BANK_COMMUNICATION entries as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Each routine performs
 * cm_interbank_complete for its entry and leaves Z at the destination, where
 * the trampoline enters the destination's own native routine. Inline CADRs come
 * from the program's compiled operand table (cm_interbank_sites for every
 * genuine BANKCALL/IBNKCALL/POSTJUMP/USPRCADR site); SUPDACAL's data word from
 * the program's passive constant reader. io is a cm_native_io. */
#include "agc_transfer.h"
#include "cm_interbank_complete.h"
extern const agc_routine_table cm_interbank_routines;
const agc_site_operand *cm_interbank_site_table(uint16_t *count);
#endif
