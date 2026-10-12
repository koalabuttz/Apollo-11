/* SPDX-License-Identifier: MIT */
#ifndef LM_P30_ROUTINES_H
#define LM_P30_ROUTINES_H
/* Luminary099 P30_P37: P30 (bank 35, 02000-02026) and S30.1 (bank 34,
 * 02000-02056) as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P30.md). io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 35):
 *   P30 (2000) TC UPFLAG (UPDATFLG); (2002) TC UPFLAG (TRACKFLG);
 *   P30N33 (2004) CAF V06N33, TC VNPOOH; (2006) CAF V06N81, TC VNPOOH;
 *   (2010) TC DOWNFLAG (UPDATFLG); (2012) TC INTPRET;
 *   PARAM30 (2017) CAF V06N42, TC VNPOOH; (2021) TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS):
 *   34,2000 S30.1: STQ QTEMP, DLOAD TIG, STCALL TDEC1 LEMPREC;
 *   34,2005 where LEMPREC returns: RTIG and VTIG from RATT and VATT (X2
 *           from RTX2, X1 from RTX1), the local-vertical frame at 0D,
 *           DELVSLV through it (VXM, VSL1) into DELVSIN, |DELVSIN| into
 *           DELVSAB, RTIG pushed, VTIG + DELVSIN, CALL PERIAPO1;
 *   34,2043 CALL SHIFTR1; 34,2045 CALL MAXCHK;
 *   34,2047 STODL HPER 4D, CALL SHIFTR1; 34,2053 CALL MAXCHK;
 *   34,2055 STCALL HAPO to C(QTEMP);
 *   35,2013 CALL S30.1; 35,2015 SET UPDATFLG, EXIT;
 *   35,2022 SETGO XDELVFLG REVN1645. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P30_PHYSICAL(bank,z) ((uint16_t)((bank)*1024u+((z)-02000u)))
#define LM_P30_B34(z) LM_P30_PHYSICAL(034u,z)
#define LM_P30_B35(z) LM_P30_PHYSICAL(035u,z)
#define LM_P30_INTERPRETIVE_ROUTINES 10u
#define LM_P30_BASIC_ROUTINES 8u
extern const agc_routine_table lm_p30_interpretive_routines;
extern const agc_routine_table lm_p30_basic_routines;
#endif
