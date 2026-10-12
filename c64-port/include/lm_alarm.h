/* SPDX-License-Identifier: MIT */
#ifndef LM_ALARM_H
#define LM_ALARM_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 ALARM_AND_ABORT as native routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Entries are the file's labels that other
 * code reaches (ALARM ... VARALARM, PRIOLARM in bank 10, FLAGS in bank 13,
 * GOPOODOO from restart group 4) and the points where control comes back
 * into the file: MULTEXIT's `INDEX A; TC 1` targets inside it, the returns
 * from BANKCALL and SUPERSW, and the erasable point ALMCADR, which executes
 * the return address CURTAINS and VARALARM store there as TC. Each routine
 * leaves Z at the next point: a caller's return, PRIODSPR, BANKCALL,
 * SUPERSW, POSTJUMP (to ENEMA), or one of its own entries. Alarm codes after
 * the callers' TCs, this file's own BANKCALL/POSTJUMP CADRs and its fixed
 * constants are compiled from the source listing (src/lm_alarm_sites.inc);
 * a program running these routines uses lm_alarm_operands as its operand
 * table. io is a p66_bias_io; WHIMPER's RESUME needs program->hardware. */
#define LM_AL_ALARM 05567u
#define LM_AL_ALARM2 05571u
#define LM_AL_BAILOUT 05634u
#define LM_AL_BAILOUT_RET 05643u   /* BAILOUT's TC BORTENT returns here (INHINT; WHIMPER) */
#define LM_AL_WHIMPER 05644u       /* = ABORT */
#define LM_AL_POODOO 05652u
#define LM_AL_ABORT2 05654u
#define LM_AL_POODOO_RET 05661u
#define LM_AL_GOPOODOO 05665u
#define LM_AL_FLAGS_RET 05670u     /* after TC BANKCALL; CADR FLAGS */
#define LM_AL_V37KLEAN_RET 05676u
#define LM_AL_MRKLEAN_RET 05700u
#define LM_AL_SUPERSW_RET 05703u
#define LM_AL_CCSHOLE 05705u
#define LM_AL_CURTAINS 05711u
#define LM_AL_CURTAINS_RET 05715u
#define LM_AL_BAILOUT1 05716u
#define LM_AL_POODOO1 05726u
#define LM_AL_ALARM1 05732u
#define LM_AL_ALMNCADR 05734u
#define LM_AL_VARALARM 05744u
#define LM_AL_VARALARM_RET 05752u
#define LM_AL_PRIOLARM 03562u      /* bank 10 */
#define LM_AL_PRIOLARM_RET 03571u  /* bank 10 */
#define LM_AL_FLAGS 03755u         /* bank 13 */
#define LM_AL_ALMCADR 01363u       /* erasable */
#define LM_AL_PRIODSPR 02504u      /* bank 10, PINBALL */
#define LM_AL_BANKCALL 04616u
#define LM_AL_POSTJUMP 04635u
#define LM_AL_SUPERSW 04727u
#define LM_AL_SWRETURN_POINT (uint16_t)(2048u+(04631u-04000u))
#define LM_AL_ROUTINES 26u
extern const agc_routine_table lm_alarm_routines;
const agc_site_operand *lm_alarm_operand_table(uint16_t *count);
#endif
