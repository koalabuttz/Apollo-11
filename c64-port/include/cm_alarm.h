/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_H
#define CM_ALARM_H
#include "agc_transfer.h"
#include "cm_native_image.h"
/* Comanche055 ALARM_AND_ABORT as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries are the file's labels that other
 * code reaches (ALARM, ALARM2, BAILOUT = ABORT, POODOO, CCSHOLE, CURTAINS,
 * VARALARM, PRIOLARM in bank 10) and the points where control comes back
 * into the file: MULTEXIT's `INDEX A; TC 1` targets inside it, the returns
 * from BANKCALL (VAC5STOR, MR.KLEAN) and DOWNFLAG, and the erasable point
 * ALMCADR, which executes the return address CURTAINS and VARALARM store
 * there as TC. Each routine leaves Z at the next point: a caller's return,
 * PRIODSPR, BANKCALL, DOWNFLAG, POSTJUMP (to ENEMA), or one of its own
 * entries. Alarm codes after the callers' TCs, this file's own
 * BANKCALL/POSTJUMP CADRs and its fixed constants are compiled from the
 * source listing (src/cm_alarm_operands.inc); a program running these
 * routines uses cm_alarm_operands as its operand table. io is a
 * cm_native_io; WHIMPER's RESUME needs program->hardware. */
#define CM_AL_ALARM 05650u
#define CM_AL_ALARM2 05652u
#define CM_AL_POODOO 05721u
#define CM_AL_POODOO_RET 05726u     /* after TC BANKCALL; CADR VAC5STOR */
#define CM_AL_POODOO_ABORT 05732u   /* ABORT2's TC BORTENT returns here */
#define CM_AL_DOWNFLAG1_RET 05740u  /* after TC DOWNFLAG; ADRES STATEFLG */
#define CM_AL_DOWNFLAG2_RET 05742u  /* REINTFLG */
#define CM_AL_DOWNFLAG3_RET 05744u  /* NODOFLAG */
#define CM_AL_MRKLEAN_RET 05746u
#define CM_AL_CCSHOLE 05747u
#define CM_AL_CCSHOLE_RET 05754u    /* after TC BANKCALL; CADR VAC5STOR */
#define CM_AL_CURTAINS 05757u
#define CM_AL_CURTAINS_RET 05763u
#define CM_AL_VARALARM 05764u
#define CM_AL_VARALARM_RET 05772u
#define CM_AL_BAILOUT 07755u        /* = ABORT */
#define CM_AL_BAILOUT_RET 07762u    /* after TC BANKCALL; CADR VAC5STOR */
#define CM_AL_WHIMPER_M1 07766u     /* BORTENT's return for BAILOUT; TC WHIMPER -1 */
#define CM_AL_PRIOLARM 03766u       /* bank 10 */
#define CM_AL_PRIOLARM_RET 03775u   /* bank 10 */
#define CM_AL_ALMCADR 01363u        /* erasable */
#define CM_AL_PRIODSPR 02726u       /* bank 10, PINBALL */
#define CM_AL_DOWNFLAG 05560u
#define CM_AL_BANKCALL 04662u
#define CM_AL_POSTJUMP 04701u
#define CM_AL_ROUTINES 21u
extern const agc_routine_table cm_alarm_routines;
const agc_site_operand *cm_alarm_operand_table(uint16_t *count);
#endif
