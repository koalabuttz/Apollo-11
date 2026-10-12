/* SPDX-License-Identifier: MIT */
#ifndef CM_V37_H
#define CM_V37_H
#include "cm_fresh_start.h"
/* Comanche055 FRESH_START_AND_RESTART, bank 04 (GOP00FIX, verb 37 with
 * R00TOP00, ISITP00, CHECKTAB, ISSERVON and CANV37, ROC/R00, POOH,
 * SEUDOP00/GROUPKIL, GOMOD/GOGOPROG, RENDV00/RENDN00/P00FIZZ/KILL20/REV37/
 * STATQUO, NOUVEAU, V37NONO, V37XEQ/V37XEQC, INITSUB/RAKE) and bank 20
 * (VAC5STOR), as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries are GOP00FIX (POSTJUMP from
 * FRESH_START), V37, CANV37 (POSTJUMP from SERVICER207), ROC and V37XEQ
 * (TEMPFLSH -3 through INITDSP), the
 * basic code after R00's interpretive EXIT, VAC5STOR (BANKCALL from
 * FRESH_START) and the return point after each TC to a routine outside the
 * file. Each routine runs until it leaves Z at one of those routines
 * (CLEARMRK, CLEARMRK +2, BANKCALL, IBNKCALL, POSTJUMP, ALARM, RELDSP,
 * PHASCHNG, INTPRET, DOWNFLAG, ENDOFJOB, FALTON, SPVAC, NEWMODEA, STOPRATE,
 * UPFLAG, GENTRAN, SWRETURN). The fixed constants and the PREMM1,
 * FCADRMM1, DNLADMM1 and FLAGTABL tables are compiled from the source
 * listing (src/cm_v37_operands.inc); the CADRs after its BANKCALL/IBNKCALL/
 * POSTJUMP are in cm_interbank_site_table, which a program running these
 * routines uses as its operand table. io is a cm_fresh_io. */
#define CM_V37_ROUTINES 36u
#define CM_V37_MMNUMBER 0775u
#define CM_V37_MINDEX 0774u
extern const agc_routine_table cm_v37_routines;
/* Entry addresses, for tests: bank (04 or 020) and Z. */
typedef struct { uint8_t bank; uint16_t z; } cm_v37_entry;
extern const cm_v37_entry cm_v37_entries[CM_V37_ROUTINES];
#endif
