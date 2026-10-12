/* SPDX-License-Identifier: MIT */
#ifndef LM_PHASE_TABLE_H
#define LM_PHASE_TABLE_H
#include "agc_transfer.h"
/* Luminary099 PHASE_TABLE_MAINTENANCE as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). io must be the
 * mission's p66_bias_io (INHINT/RELINT, ROR SUPERBNK).
 *
 * Registered entries: NEWMODEX, NEWMODEA and MMDSPLAY (to TCF BANKJUMP with
 * the CADR of SETUPDSP), CHECKMM (to Q+1 or Q+2 in INTERPRETER), 2PHSCHNG,
 * PHASCHNG and PHSCHNGA (phase word in A; to their DTCB into PHSCHNG2),
 * SETUPDSP (to TC NOVAC, 2CADR DSPMMJOB) and its NOVAC return 10,2221 (DXCH Z
 * to the caller), PHSCHNG2 (type C, or types A/B through ONEORTWO to the DTCB
 * into CON2) and CON2; PHSCHNG2 and CON2 end in the DTCB to the caller's
 * return. Inline operands after every TC NEWMODEX/CHECKMM/PHASCHNG/2PHSCHNG,
 * and the 2CADR DSPMMJOB after SETUPDSP's TC NOVAC, are in
 * lm_phase_table_operands. The same instruction sequence as Comanche055
 * (include/cm_phase_table.h) apart from the PHSCHNGA entry. */
#define LM_PT_NEWMODEX 05311u
#define LM_PT_NEWMODEA 05314u
#define LM_PT_MMDSPLAY 05315u
#define LM_PT_CHECKMM 05321u
#define LM_PT_2PHSCHNG 05327u
#define LM_PT_PHASCHNG 05353u
#define LM_PT_PHSCHNGA 05357u
#define LM_PT_SETUPDSP 02213u        /* bank 10 */
#define LM_PT_SETUPDSP_RETURN 02221u /* bank 10 */
#define LM_PT_PHSCHNG2 02224u        /* bank 10 */
#define LM_PT_CON2 02312u            /* bank 10 */
#define LM_PT_BANKJUMP 04640u
#define LM_PT_NOVAC 05072u
#define LM_PT_Q_PLUS_1 06737u
#define LM_PT_Q_PLUS_2 06741u        /* NEG */
#define LM_PT_ROUTINES 11u
extern const agc_routine_table lm_phase_table_routines;
extern const agc_site_operand *const lm_phase_table_operands;
#define LM_PT_OPERAND_COUNT 181u     /* LM_PHASE_SITE_WORDS */
#endif
