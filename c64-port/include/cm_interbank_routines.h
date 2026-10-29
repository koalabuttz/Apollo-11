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
/* BANKCALL, SWCALL -1, SWCALL, SWRETURN and POSTJUMP (src/cm_interbank_idle.c):
 * cm_interbank_routines' first five entries, also as a table of their own for
 * the flight store to pack beside another module (not a routine table of the
 * program: the same keys are cm_interbank_routines'). */
enum exec_status cm_ib_bankcall(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status cm_ib_swcall_m1(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status cm_ib_swcall(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status cm_ib_swreturn(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status cm_ib_postjump(const agc_native_program *,apollo_overlay_state *,void *);
extern const agc_routine_table cm_interbank_idle_chunk;
#endif
