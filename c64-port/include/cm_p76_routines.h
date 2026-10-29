/* SPDX-License-Identifier: MIT */
#ifndef CM_P76_ROUTINES_H
#define CM_P76_ROUTINES_H
/* Comanche055 P76 (bank 13, 02071-02230) as routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md, docs/CM-P76.md). io is
 * a p66_bias_io.
 *
 * Basic (keyed by Z in bank 13):
 *   P76 (2071) TC UPFLAG (TRACKFLG);
 *   (2073) CAF V06N84, TC BANKCALL (CADR GOFLASH); its returns (2076) TCF
 *          ENDP76, (2077) TC +2, (2100) TC -5;
 *   (2101) CAF V06N84 +1, TC BANKCALL (CADR GOFLASH); its returns (2104)
 *          TCF ENDP76, (2105) TC +2, (2106) TC -5; (2107) TC INTPRET;
 *   (2175) TC PHASCHNG (OCT 04024); (2177) TC UPFLAG (REINTFLG); (2201) TC
 *          INTPRET; (2207) TC BANKCALL (CADR INTWAKE1);
 *   ENDP76 (2212) ZERO into MARKCTR and VHFCNT, NEGONE into MRKBUF2, TCF
 *          GOTOPOOH.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 13):
 *   (2110) DLOAD TIG, STCALL TDEC1 OTHPREC;
 *   COMPMAT (2114) the local frame from RATT and VATT at 12D, DELVOV
 *          through it (VXM, VSL1) plus VATT at 6D, CALL INTSTALL;
 *   (2136) CALL P76SUB1;
 *   (2140) VCV and RCV scaled by X2 (VSR* 0,2), TET, CLEAR INTYPFLG,
 *          DLOAD TETTHIS, INTOTHIS STCALL TDEC1 INTEGRVS;
 *   (2157) CALL INTSTALL; (2161) CALL P76SUB1;
 *   (2163) RRECT, RCV, TET from RATT1, TAT and VATT1, CALL MINIRECT;
 *   (2174) EXIT; (2202) CALL ATOPOTH; (2204) SSP QPRET OUT, EXIT;
 *   OUT (2211) EXIT;
 *   P76SUB1 (2222) CLEAR MOONFLAG, SLOAD X2, BHIZ +2, else SET MOONFLAG,
 *          RVQ. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define CM_P76_B13(z) ((uint16_t)(013u*1024u+((z)-02000u)))
#define CM_P76_INTERPRETIVE_ROUTINES 12u
#define CM_P76_BASIC_ROUTINES 15u
extern const agc_routine_table cm_p76_interpretive_routines;
extern const agc_routine_table cm_p76_basic_routines;
#endif
