/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435B_ROUTINES_H
#define LM_S3435B_ROUTINES_H
/* Luminary099 P34-35_P74-75 S34/35.3 and the integration interface
 * GOINT/INTINT as interpretive routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-S3435B.md), keyed by LOC under
 * BANKSET at NEWOPS in bank 35:
 *   S34/35.3 (3377)  STQ NORMEX, CALL LOMAT (lm-s3435);
 *            (3402)  where LOMAT returns: DELVLVC through the matrix at 0D
 *                    (VXM) into DELVEET3, the integration arguments pushed,
 *                    CALL INTINT;
 *            (3421)  where INTINT returns: STORE RATT in RTARG, on into
 *                    NOVRWRT (lm-s3435);
 *   GOINT    (3462)  pushes ZEROVECS and NOMTPI twice, on into INTINT;
 *   INTINT   (3466)  STQ RTRN, CALL INTSTALL (bank 13, not ported);
 *            (3471)  where INTSTALL returns: the integration type, TDEC1,
 *                    MOONFLAG from RTX2, the scaled state into RCV/VCV,
 *                    STCALL VCV INTEGRVS (bank 13, not ported);
 *            (3517)  where INTEGRVS returns: VLOAD RATT, GOTO C(RTRN).
 * io is a p66_bias_io. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_S3435B_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_S3435B_ROUTINES 7u
extern const agc_routine_table lm_s3435b_routines;
#endif
