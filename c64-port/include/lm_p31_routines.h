/* SPDX-License-Identifier: MIT */
#ifndef LM_P31_ROUTINES_H
#define LM_P31_ROUTINES_H
/* Luminary099 LAMBERT_AIMPOINT_GUIDANCE (P31 and REVN1645, bank 35,
 * 02450-02544) as routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P31.md). io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 35):
 *   P31 (2450) TC P20FLGON; (2451) CAF V06N33, TC VNPOOH; (2453) TC
 *   INTPRET; after the EXIT at 2532: (2533) CAF V06N81, TC VNPOOH; (2535)
 *   CAF V06N42, TC VNPOOH; (2537) TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 35):
 *   (2454) CLEAR UPDATFLG, DLOAD TIG, STCALL TDEC1 LEMPREC;
 *   (2461) where LEMPREC returns: SETPD 0D, RTIG and RINIT from RATT,
 *          VTIG and VINIT from VATT, P30ZERO pushed (E4 and NUMIT), DELLT4
 *          pushed, TPASS4 = TIG + DELLT4 (X1 from RTX1), X2 from RTX2,
 *          CALL INITVEL;
 *   (2503) where INITVEL returns: DELVEET3 pushed and into DELVSIN,
 *          |DELVSIN| with XDELVFLG cleared, STCALL VGDISP GET.LVC;
 *   (2512) where GET.LVC returns: RTIG pushed, VIPRIME, CALL PERIAPO1;
 *   (2517) CALL SHIFTR1; (2521) CALL MAXCHK; (2523) STODL HPER 4D, CALL
 *          SHIFTR1; (2527) CALL MAXCHK; (2531) STORE HAPO, EXIT;
 *   REVN1645 (2540) SET FINALFLG, CALL VN1645; (2543) where VN1645
 *          returns: GOTO REVN1645. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P31_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_P31_INTERPRETIVE_ROUTINES 11u
#define LM_P31_BASIC_ROUTINES 6u
extern const agc_routine_table lm_p31_interpretive_routines;
extern const agc_routine_table lm_p31_basic_routines;
#endif
