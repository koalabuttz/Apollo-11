/* SPDX-License-Identifier: MIT */
#ifndef LM_V37_H
#define LM_V37_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 FRESH_START_AND_RESTART, bank 04: GOP00FIX, verb 37 (V37,
 * CHECKTAB, ISSERVON, V37RET, CANV37), R00 and POOH, SEUDOP00, GOMOD,
 * RENDV00/RENDN00/KILL2/STATQUO, NOUVEAU, V37NONO, V37XEQ/V37XEQC and
 * SETUP70/71, as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries are GOP00FIX, V37, V37RET, R00,
 * V37XEQ, V37XEQC and the return point after each TC to a routine outside
 * the bank. Each routine runs until it leaves Z at one of those routines
 * (DOWNFLAG, CLEARMRK +2, BANKCALL, ALARM, RELDSP, POSTJUMP, ENDOFJOB,
 * PHSCHNGA, PHASCHNG, INTPRET, CLRADMOD, IBNKCALL, FALTON, SPVAC,
 * NEWMODEA) or at P70/P71 (DTCB). The CADRs after the bank's TC BANKCALL,
 * POSTJUMP and IBNKCALL, its fixed constants and the PREMM1, FCADRMM1 and
 * DNLADMM1 tables are compiled from the source listing
 * (src/lm_v37_operands.inc); a program running these routines uses
 * lm_v37_operands as its operand table. io is a p66_bias_io. */
#define LM_V37_ROUTINES 32u
#define LM_V37_MMNUMBER 0775u
#define LM_V37_MINDEX 0774u
extern const agc_routine_table lm_v37_routines;
/* Entry Z addresses (bank 04), for tests. */
extern const uint16_t lm_v37_entries[LM_V37_ROUTINES];
#endif
