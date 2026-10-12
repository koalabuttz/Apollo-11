/* SPDX-License-Identifier: MIT */
#ifndef LM_P33_ROUTINES_H
#define LM_P33_ROUTINES_H
/* Luminary099 P32-P35_P72-P75: P33/P73, AVFLAGA, AVFLAGP, P20FLGON and
 * DISDVLVC (bank 35, 02216-02407) as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P33.md). io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 35):
 *   P33 (2216) TC AVFLAGA; (2217) TC P33/P73A; P73 (2220) TC AVFLAGP;
 *   P33/P73A (2221) TC P20FLGON; (2222) CAF V06N13, TC VNPOOH; (2224) TC
 *   INTPRET; after the EXIT at 2265: (2266) TC ALARM (OCT 611); (2270) CAF
 *   V05N09, TC BANKCALL (CADR GOFLASH); its returns (2273) TC GOTOPOOH,
 *   (2274) TC +2, (2275) TC P33/P73A; (2276) TC INTPRET; after the EXIT at
 *   2334: (2335) CAF V06N75, TC VNPOOH; (2337) TC INTPRET;
 *   AVFLAGA (2347) QXCH SUBEXIT, TC UPFLAG (AVFLAG); (2353) TC SUBEXIT;
 *   AVFLAGP (2354) QXCH SUBEXIT, TC DOWNFLAG (AVFLAG); (2360) TC SUBEXIT;
 *   P20FLGON (2361) QXCH SUBEXIT, TC UPFLAG (UPDATFLG); (2365) TC UPFLAG
 *   (TRACKFLG); (2367) TC SUBEXIT;
 *   after DISDVLVC's EXIT: (2403) CA VERBNOUN, TC VNPOOH; (2405) TC
 *   INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 35):
 *   (2225) TTPI0 into TTPI, TCDH into TIG, CALL SELECTMU;
 *   P33/P73B (2233) CALL ADVANCE; (2235) CALL CDHMVR; (2237) SETPD 0D,
 *   VACT3 and RACT2 pushed, CALL INTINT3P; (2245) CALL ACTIVE; (2247)
 *   SETPD 0D, VPASS2 and RPASS2 pushed, CALL INTINT3P; (2255) CALL PASSIVE;
 *   (2257) SET ITSWICH, P30ZERO into NOMTPI, CALL S33/34.1; (2264) BZE
 *   P33/P73C, else EXIT; (2277) P30ZERO into NOMTPI, on into P33/P73C;
 *   P33/P73C (2302) BON FINALFLG P33/P73D, else SET UPDATFLG;
 *   P33/P73D (2306) TTPI + NOMTPI into TTPI, minus TCDH;
 *   P33/P73E (2314) 60MIN taken off while positive, T1TOT2, TTPI - TTPI0
 *   pushed; P33/P73F (2325) the same on its magnitude, the sign restored
 *   into T2TOT3, EXIT;
 *   (2340) DELVEET2, CALL S32/33.1; (2343) STCALL DELVEET2 VN1645; (2345)
 *   GOTO P33/P73B;
 *   DISDVLVC (2370) STORE DELVLVC, STQ NORMEX, CALL S32/33.X; (2374)
 *   DELVLVC through the matrix at 0D (MXV, VSL1), X1 into VERBNOUN, STORE
 *   DELVLVC, EXIT; (2406) GOTO C(NORMEX). */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P33_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_P33_INTERPRETIVE_ROUTINES 20u
#define LM_P33_BASIC_ROUTINES 23u
extern const agc_routine_table lm_p33_interpretive_routines;
extern const agc_routine_table lm_p33_basic_routines;
#endif
