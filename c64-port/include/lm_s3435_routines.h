/* SPDX-License-Identifier: MIT */
#ifndef LM_S3435_ROUTINES_H
#define LM_S3435_ROUTINES_H
/* Luminary099 P34-35_P74-75 rendezvous-targeting subroutines as interpretive
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md,
 * docs/LM-S3435.md), keyed by LOC under BANKSET at NEWOPS in bank 35:
 *   S34/35.1 (3274)  the line of sight ULOS and orbit normal UNRM; RVQ;
 *   S34/35.4 (3442)  STQ NORMEX, SETPD 0D, GOTO NOVRWRT;
 *   NOVRWRT  (3424)  DELVEET3 in the line-of-sight frame (DVLOS) through the
 *                    LOMAT-built matrix at 0D; STCALL DVLOS to C(NORMEX);
 *   LOMAT    (3447)  the local-vertical matrix at 0D-17D; RVQ.
 * Each runs the source from its NEWOPS to the next NEWOPS outside the file's
 * routines (the caller's return), or to the UNIT alarm (POODOO1, 1302).
 * io is a p66_bias_io. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_S3435_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_S3435_ROUTINES 4u
extern const agc_routine_table lm_s3435_routines;
#endif
