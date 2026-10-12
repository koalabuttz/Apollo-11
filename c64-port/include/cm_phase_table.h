/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_TABLE_H
#define CM_PHASE_TABLE_H
#include "agc_transfer.h"
/* Comanche055 PHASE_TABLE_MAINTENANCE as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). io must be the
 * mission's cm_native_io (INHINT/RELINT, ROR SUPERBNK).
 *
 * Registered entries: NEWMODEX, NEWMODEA and MMDSPLAY (to TCF BANKJUMP with
 * the CADR of SETUPDSP), CHECKMM (to Q+1 or Q+2 in INTERPRETER), 2PHSCHNG and
 * PHASCHNG (to their DTCB into PHSCHNG2), SETUPDSP (to TC NOVAC, 2CADR
 * DSPMMJOB) and its NOVAC return 10,2446 (DXCH Z to the caller), PHSCHNG2
 * (type C, or types A/B through ONEORTWO to the DTCB into CON2) and CON2;
 * PHSCHNG2 and CON2 end in the DTCB to the caller's return. Inline operands
 * after every TC NEWMODEX/CHECKMM/PHASCHNG/2PHSCHNG, and the 2CADR DSPMMJOB
 * after SETUPDSP's TC NOVAC, are in cm_phase_table_operands. */
#define CM_PT_NEWMODEX 05354u
#define CM_PT_NEWMODEA 05357u
#define CM_PT_MMDSPLAY 05360u
#define CM_PT_CHECKMM 05364u
#define CM_PT_2PHSCHNG 05372u
#define CM_PT_PHASCHNG 05412u
#define CM_PT_SETUPDSP 02440u        /* bank 10 */
#define CM_PT_SETUPDSP_RETURN 02446u /* bank 10 */
#define CM_PT_PHSCHNG2 02451u        /* bank 10 */
#define CM_PT_CON2 02537u            /* bank 10 */
#define CM_PT_BANKJUMP 04704u
#define CM_PT_NOVAC 05134u
#define CM_PT_Q_PLUS_1 06706u
#define CM_PT_Q_PLUS_2 06710u        /* NEG */
#define CM_PT_ROUTINES 10u
extern const agc_routine_table cm_phase_table_routines;
extern const agc_site_operand *const cm_phase_table_operands;
#define CM_PT_OPERAND_COUNT 229u     /* CM_PHASE_SITE_WORDS */
#endif
