/* SPDX-License-Identifier: MIT */
#ifndef CM_UPDATE_H
#define CM_UPDATE_H
#include "agc_transfer.h"
#include "cm_native_image.h"
/* Comanche055 UPDATE_PROGRAM (P27) as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Entries are the
 * extended verbs V70-V73 (bank 43), the POSTJUMP targets UPPART2 and
 * UPERROUT+2, the job UPJOB, the restart points UPOUT+1 and TIMEDIDR, the
 * basic code after UPJOB's interpretive EXIT, and the return point after
 * every TC to code outside the file (TESTXACT, PHASCHNG, NEWMODEX, GOXDSPF
 * through BANKCALL with its three returns, FALTON, FINDVAC, UPFLAG,
 * TPAGREE, INTWAKEU, NEWMODEX+3). Each routine runs until it leaves Z at
 * such an external entry, ENDOFJOB, ENDEXT or INTPRET. The fixed constants
 * are compiled from the source listing (src/cm_update_operands.inc); the
 * CADRs after the file's BANKCALL/POSTJUMP are in cm_interbank_site_table,
 * which a program running these routines uses as its operand table. io is a
 * cm_native_io; UPACTOFF's WAND DSALMOUT needs program->hardware (channel 11). */
#define CM_UP_ROUTINES 35u
#define CM_UP_UPTEMP 0330u
#define CM_UP_COMPNUMB 0300u
#define CM_UP_UPVERB 0302u
#define CM_UP_UPCOUNT 0303u
#define CM_UP_UPBUFF 0304u
extern const agc_routine_table cm_update_routines;
/* Entry addresses, for tests: bank (043 or 027) and Z. */
typedef struct { uint8_t bank; uint16_t z; } cm_up_entry;
extern const cm_up_entry cm_up_entries[CM_UP_ROUTINES];
#endif
