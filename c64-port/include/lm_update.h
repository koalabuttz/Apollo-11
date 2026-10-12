/* SPDX-License-Identifier: MIT */
#ifndef LM_UPDATE_H
#define LM_UPDATE_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 UPDATE_PROGRAM (P27) as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Entries are the
 * extended verbs V70-V73 (bank 43), the POSTJUMP targets UPPART2 and
 * UPERROUT+2, the job UPJOB, the restart points UPOUT+1 and TIMEDIDR, the
 * basic code after UPJOB's interpretive EXIT, and the return point after
 * every TC to code outside the file (TESTXACT, PHASCHNG, NEWMODEX, GOXDSPF
 * through BANKCALL with its three returns, FALTON, FINDVAC, UPFLAG,
 * TPAGREE, INTWAKEU, NEWMODEX+3). Each routine runs until it leaves Z at
 * such an external entry, ENDOFJOB, ENDEXT or INTPRET. The CADRs after the
 * file's own BANKCALL/POSTJUMP and its fixed constants are compiled from the
 * source listing (src/lm_update_operands.inc); a program running these
 * routines uses lm_update_operands as its operand table. io is a
 * p66_bias_io; UPACTOFF's WAND DSALMOUT needs program->hardware (channel 11). */
#define LM_UP_ROUTINES 35u
#define LM_UP_UPTEMP 01167u
#define LM_UP_UPVERB 01172u
#define LM_UP_UPCOUNT 01173u
#define LM_UP_COMPNUMB 01170u
#define LM_UP_UPBUFF 01174u
extern const agc_routine_table lm_update_routines;
/* Entry addresses, for tests: bank (043 or 04) and Z. */
typedef struct { uint8_t bank; uint16_t z; } lm_up_entry;
extern const lm_up_entry lm_up_entries[LM_UP_ROUTINES];
#endif
