/* SPDX-License-Identifier: MIT */
#ifndef LM_P21_ROUTINES_H
#define LM_P21_ROUTINES_H
/* Luminary099 GROUND_TRACKING_DETERMINATION_PROGRAM (P21, bank 24,
 * 03505-03667) as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P21.md). io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 24):
 *   PROG21 (3505) ONE into OPTION2, on into (3507) CAF BIT2, TC BANKCALL
 *          (CADR GOPERF4); its returns (3512) TC GOTOPOOH, (3513) TC +2,
 *          (3514) TC -5;
 *   P21PROG1 (3515) CAF V6N34, TC BANKCALL (CADR GOFLASH); its returns
 *          (3520) TC GOTOPOOH, (3521) TC +2, (3522) TC -5; (3523) TC
 *          INTPRET;
 *   after the EXIT at 3645: (3646) CAF V06N43, TC BANKCALL (CADR GOFLASH);
 *          its returns (3651) and (3652) TC GOTOPOOH, (3653) TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 24):
 *   (3524) DLOAD DSPTEM1, STCALL TDEC1 INTSTALL;
 *   (3530) BON P21FLAG P21CONT, else CLEAR VINTFLAG, SLOAD SR1 OPTION2,
 *          BHIZ +2, else SET VINTFLAG; (3541) CLEAR DIM0FLAG and INTYPFLG,
 *          CALL INTEGRV; (3546) GOTO P21VSAVE;
 *   P21CONT (3550) RCV, VCV and TET from the base state, CLEAR DIM0FLAG and
 *          MOONFLAG, SET MOONFLAG unless P21ORIG is zero, CALL INTEGRVS;
 *   P21VSAVE (3571) the base state from TAT, RATT1 and VATT1, P21VEL
 *          (ABVAL, SL* 0,2), UNIT RATT DOT VATT, DDV P21VEL, ASIN (to
 *          ARCSIN);
 *   (3610) STORE P21GAM, SXA,2 P21ORIG, SLOAD SR1 OPTION2, BHIZ to BON
 *          SURFFLAG P21DSP, SET P21FLAG, on into P21DSP
 *          (3625) LUNAFLAG by X2, RATT into ALPHAV, TAT, CLEAR ERADFLAG,
 *          CALL LAT-LONG; (3642) DMP K.01, STORE P21ALT, EXIT;
 *   (3654) P21TIME + 600SEC into DSPTEM1, RTB P21PROG1. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P21_B24(z) ((uint16_t)(024u*1024u+((z)-02000u)))
#define LM_P21_INTERPRETIVE_ROUTINES 10u
#define LM_P21_BASIC_ROUTINES 14u
extern const agc_routine_table lm_p21_interpretive_routines;
extern const agc_routine_table lm_p21_basic_routines;
#endif
