/* SPDX-License-Identifier: MIT */
#ifndef CM_P21_ROUTINES_H
#define CM_P21_ROUTINES_H
/* Comanche055 GROUND_TRACKING_DETERMINATION_PROGRAM (P21, bank 37,
 * 02001-02154) as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/CM-P21.md). io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 37):
 *   PROG21 (2001) ONE into OPTION2, on into (2003) CAF BIT2, TC BANKCALL
 *          (CADR GOPERF4); its returns (2006) TC GOTOPOOH, (2007) TC +2,
 *          (2010) TC -5;
 *   P21PROG1 (2011) CAF V6N34, TC BANKCALL (CADR GOFLASH); its returns
 *          (2014) TC GOTOPOOH, (2015) TC +2, (2016) TC -5; (2017) TC
 *          INTPRET;
 *   after the EXIT at 2130: (2131) CAF V06N43, TC BANKCALL (CADR GOFLASH);
 *          its returns (2134) and (2135) TC GOTOPOOH, (2136) TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 37):
 *   (2020) DLOAD DSPTEM1, STCALL TDEC1 INTSTALL;
 *   (2024) BON P21FLAG P21CONT, else SET VINTFLAG, SLOAD SR1 OPTION2, BHIZ
 *          +2, else CLEAR VINTFLAG; (2035) CLEAR DIM0FLAG and INTYPFLG,
 *          CALL INTEGRV; (2042) GOTO P21VSAVE;
 *   P21CONT (2044) RCV, VCV and TET from the base state, CLEAR DIM0FLAG and
 *          MOONFLAG, SET MOONFLAG unless P21ORIG is zero, CALL INTEGRVS;
 *   P21VSAVE (2065) the base state from TAT, RATT1 and VATT1, P21VEL
 *          (ABVAL, SL* 0,2), UNIT RATT DOT VATT, DDV P21VEL, ASIN (to
 *          ARCSIN);
 *   (2104) STORE P21GAM, SXA,2 P21ORIG, SET P21FLAG, on into P21DSP
 *          (2110) LUNAFLAG by X2, RATT into ALPHAV, TAT, CLEAR ERADFLAG,
 *          CALL LAT-LONG; (2125) DMP K.01, STORE P21ALT, EXIT;
 *   (2137) P21TIME + 600SEC into DSPTEM1, RTB P21PROG1. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define CM_P21_B37(z) ((uint16_t)(037u*1024u+((z)-02000u)))
#define CM_P21_INTERPRETIVE_ROUTINES 10u
#define CM_P21_BASIC_ROUTINES 14u
extern const agc_routine_table cm_p21_interpretive_routines;
extern const agc_routine_table cm_p21_basic_routines;
#endif
