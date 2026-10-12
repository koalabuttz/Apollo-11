/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERBANK_ROUTINES_H
#define LM_INTERBANK_ROUTINES_H
/* Luminary099 INTER-BANK_COMMUNICATION entries as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Each routine performs
 * lm_interbank for its entry and leaves Z at the destination, where the
 * trampoline enters the destination's own native routine. Inline CADRs come
 * from the program's compiled operand table (lm_interbank_sites for every
 * genuine BANKCALL/IBNKCALL/POSTJUMP/USPRCADR site); SUPDACAL's data word from
 * the program's passive constant reader. io is a p66_bias_io. */
#include "agc_transfer.h"
#include "lm_interbank.h"
extern const agc_routine_table lm_interbank_routines;
const agc_site_operand *lm_interbank_site_table(uint16_t *count);
#endif
