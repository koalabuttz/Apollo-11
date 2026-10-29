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
/* BANKCALL, SWCALL -1, SWCALL, SWRETURN and POSTJUMP (src/lm_interbank_idle.c):
 * lm_interbank_routines' first five entries, also as a table of their own for
 * the flight store to pack beside another module (not a routine table of the
 * program: the same keys are lm_interbank_routines'). */
enum exec_status lm_ib_bankcall(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ib_swcall_m1(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ib_swcall(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ib_swreturn(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ib_postjump(const agc_native_program *,apollo_overlay_state *,void *);
extern const agc_routine_table lm_interbank_idle_chunk;
#endif
