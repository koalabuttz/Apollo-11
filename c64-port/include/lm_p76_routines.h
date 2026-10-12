/* SPDX-License-Identifier: MIT */
#ifndef LM_P76_ROUTINES_H
#define LM_P76_ROUTINES_H
/* Luminary099 P76 (bank 13, 02207-02350) as routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P76.md). io is
 * a p66_bias_io.
 *
 * Basic (keyed by Z in bank 13):
 *   P76 (2207) TC UPFLAG (TRACKFLG); (2211) TC INTPRET;
 *   (2216) CAF V06N84, TC BANKCALL (CADR GOFLASH); its returns (2221) TCF
 *          ENDP76, (2222) TC +2, (2223) TC -5;
 *   (2224) CAF V06N84 +1, TC BANKCALL (CADR GOFLASH); its returns (2227)
 *          TCF ENDP76, (2230) TC +2, (2231) TC -5; (2232) TC INTPRET;
 *   (2316) TC PHASCHNG (OCT 04024); (2320) TC UPFLAG (REINTFLG); (2322) TC
 *          INTPRET; (2330) TC BANKCALL (CADR INTWAKE1);
 *   ENDP76 (2333) CAF ZERO, TS MARKCTR, TCF GOTOPOOH.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 13):
 *   (2212) VLOAD DELVLVC, STORE DELVOV, EXIT;
 *   (2233) DLOAD TIG, STCALL TDEC1 OTHPREC;
 *   COMPMAT (2237) the local frame from RATT and VATT at 12D, DELVOV
 *          through it (VXM, VSL1) plus VATT at 6D, CALL INTSTALL;
 *   (2261) CALL P76SUB1;
 *   (2263) VCV and RCV scaled by X2 (VSR* 0,2), TET, CLEAR INTYPFLG,
 *          DLOAD TETTHIS, INTOTHIS STCALL TDEC1 INTEGRVS;
 *   (2302) CALL INTSTALL;
 *   (2304) RRECT, RCV, TET from RATT1, TAT and VATT1, CALL MINIRECT;
 *   (2315) EXIT; (2323) CALL ATOPOTH; (2325) SSP QPRET OUT, EXIT;
 *   OUT (2332) EXIT;
 *   P76SUB1 (2340) AXT,2 2, SET MOONFLAG, BON CMOONFLG to C(QPRET), else
 *          AXT,2 0, CLEAR MOONFLAG, RVQ. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P76_B13(z) ((uint16_t)(013u*1024u+((z)-02000u)))
#define LM_P76_INTERPRETIVE_ROUTINES 12u
#define LM_P76_BASIC_ROUTINES 16u
extern const agc_routine_table lm_p76_interpretive_routines;
extern const agc_routine_table lm_p76_basic_routines;
#endif
