/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435C_ROUTINES_H
#define LM_S3435C_ROUTINES_H
/* Luminary099 P34-35_P74-75 S34/35.2's integration head and LOMAT tail and
 * S34/35.5 as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-S3435C.md). io is a p66_bias_io.
 *
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 35):
 *   S34/35.2 (3306)  STQ SUBEXIT, the integration arguments pushed (VPASS3,
 *                    RPASS3, INTIME, TPASS4, TWOPI, NN, and with NN nonzero
 *                    ZEROVECS), CALL INTINT (lm-s3435b);
 *            (3367)  where INITVEL returns: CALL LOMAT (lm-s3435);
 *            (3371)  where LOMAT returns: DELVEET3 through the matrix at 0D
 *                    (MXV) into DELVLVC, GOTO C(SUBEXIT);
 *   S34/35.5 (3522)  STQ SUBEXIT; BON FINALFLG FLAGON, else SET UPDATFLG,
 *                    GOTO FLAGOFF;
 *   FLAGON   (3531)  CLEAR NTARGFLG, DELVLVC into GDT/2, EXIT;
 *            (3566)  where the target check's INTPRET returns: BOFF NTARGFLG
 *                    NOCHG, else CALL S34/35.3 (lm-s3435b);
 *   NOCHG    (3572)  CLEAR XDELVFLG, DELVEET3 into DELVSIN, on into FLAGOFF;
 *   FLAGOFF  (3576)  CALL S34/35.4 (lm-s3435);
 *            (3600)  where S34/35.4 returns: EXIT;
 *            (3604)  where VNPOOH's INTPRET returns: GOTO C(SUBEXIT).
 * Basic (bank 35):
 *   FLAGON +5 (3536)        CAF V06N81, TC BANKCALL with CADR GOFLASH;
 *   GOFLASH's returns       terminate (3541) TC GOTOPOOH; proceed (3542)
 *                           TC +2 into the target check; recycle (3543)
 *                           TC FLAGON +5;
 *   +2 (3544)               EBANK7, then the NTARGCHK loop sums
 *                           GDT/2 - DELVLVC over six words into L; nonzero
 *                           TC UPFLAG with ADRES NTARGFLG, zero TC INTPRET;
 *   (3565)                  where UPFLAG returns: TC INTPRET;
 *   (3601)                  after FLAGOFF's EXIT: CAF V06N59, TC VNPOOH
 *                           (lm-p34);
 *   (3603)                  where VNPOOH returns: TC INTPRET.
 * S34/35.2's Lambert body (3327-3366: ACOS) waits for the interpretive
 * ARCCOS body. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_S3435C_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_S3435C_INTERPRETIVE_ROUTINES 10u
#define LM_S3435C_BASIC_ROUTINES 8u
extern const agc_routine_table lm_s3435c_interpretive_routines;
extern const agc_routine_table lm_s3435c_basic_routines;
#endif
