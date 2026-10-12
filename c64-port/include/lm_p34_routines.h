/* SPDX-License-Identifier: MIT */
#ifndef LM_P34_ROUTINES_H
#define LM_P34_ROUTINES_H
/* Luminary099 P34-35_P74-75 as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P34-TRANSFER.md). io is a
 * p66_bias_io.
 *
 * Basic (bank 35): GET45 +1 and the points where VN1645's display tail
 * resumes after COMPTGO, DELAYJOB and GOFLASH; KILCLOCK, N45PROC, CLUPDATE
 * and their PHASCHNG/UPFLAG returns; DISPLAYE, VNPOOH and P3XORP7X with the
 * three GOFLASH returns of each display. Each runs the source from its entry
 * to the point where control leaves the file and leaves Z, Q, A, L and the
 * banks as the original does. TC NORMEX and TC RTRN run the return TC that
 * DISPLAYE and VNPOOH stored there, inline: the routine refuses a stored word
 * that is not a TC into fixed memory.
 *
 * Interpretive (keyed by LOC under BANKSET at NEWOPS): VN1645's three NEWOPS
 * frontiers (35,3606, 3630, 3634), CLUPDATE's CLEAR GOTO (35,3671),
 * GET+MGA (06,3717), GET.LVC (06,3733) and SELECTMU (10,2000). Each runs the
 * certified lm-vn1645/lm-midgim step from its frontier to the next. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P34_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_P34_B06(z) ((uint16_t)(06u*1024u+((z)-02000u)))
#define LM_P34_B10(z) ((uint16_t)(010u*1024u+((z)-02000u)))
#define LM_P34_BASIC_ROUTINES 20u
#define LM_P34_INTERPRETIVE_ROUTINES 7u
extern const agc_routine_table lm_p34_basic_routines;
extern const agc_routine_table lm_p34_interpretive_routines;
/* GET45 +1 (where VN1645's interpretive routines end) and the interpretive
 * routines themselves, for programs (the C64 diagnostics) that register
 * only some of them. */
enum exec_status lm_p34_get45_comptgo(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_getmga(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_getlvc(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_selectmu(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_vn1645(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_vn1645_mga(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_get45(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_p34_clupdate(const agc_native_program *,apollo_overlay_state *,void *);
/* The file's inline operands that the generated INTER-BANK site table
 * leaves out: the CADR GOFLASH after DISPLAYE's and VNPOOH's TCR BANKCALL
 * (35,3700 and 35,3717). A program merges them with lm_interbank_sites. */
#define LM_P34_SITES 2u
const agc_site_operand *lm_p34_site_table(uint16_t *count);
#endif
