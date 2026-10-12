/* SPDX-License-Identifier: MIT */
#ifndef LM_FRESH_START_H
#define LM_FRESH_START_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 FRESH_START_AND_RESTART, bank 05 and fixed-fixed (fresh start,
 * hardware and software restart, the restart-group scan, ENEMA and
 * GOTOPOOH), as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries: SLAP1, MR.KLEAN, P00KLEAN,
 * V37KLEAN, GOPROG, ENEMA, GOPROG2, GOTOPOOH and the return points after
 * TC MMDSPLAY, TC SWCALL (RESTARTS) and TC ALARM (PTBAD). Each routine runs
 * until it leaves Z at MMDSPLAY, SWCALL, ALARM or POSTJUMP (DUMMYJOB +2,
 * GOP00FIX), or at the fixed-fixed return of a BANKCALL'd KLEAN entry. The
 * CADRs after the file's own POSTJUMPs and its fixed constants are compiled
 * from the source listing (src/lm_fresh_start_operands.inc); a program
 * running these routines uses lm_fresh_operands as its operand table.
 * io is an lm_fresh_io (p66_bias_io first, so lm_interbank_routines read
 * the same record); channel 11 is program->hardware's. */
typedef struct {
 p66_bias_io control;
 agc_word channel5,channel6,channel12,channel13,channel14;  /* outputs this file writes */
 agc_word channel15,channel16,channel33;                    /* inputs: MNKEYIN, NAVKEYIN, CHAN33 */
 uint16_t trap31a,trap31b,trap32;                           /* HANDRUPT traps armed by channel 13 */
} lm_fresh_io;
#define LM_FS_ROUTINES 11u
#define LM_FS_ERESTORE 01360u
#define LM_FS_SKEEP4 01374u
#define LM_FS_SKEEP5 01375u
#define LM_FS_SKEEP7 01377u
extern const agc_routine_table lm_fresh_start_routines;
/* Entry Z addresses (bank 05, GOTOPOOH fixed-fixed), for tests. */
extern const uint16_t lm_fresh_entries[LM_FS_ROUTINES];
#endif
