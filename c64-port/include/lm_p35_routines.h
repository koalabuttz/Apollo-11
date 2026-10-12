/* SPDX-License-Identifier: MIT */
#ifndef LM_P35_ROUTINES_H
#define LM_P35_ROUTINES_H
/* Luminary099 P34-35_P74-75 P35/P75 (bank 35, 02725-02763) as routines of
 * the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md, docs/LM-P35.md).
 * io is a p66_bias_io.
 *
 * Basic (keyed by Z in bank 35):
 *   P35       (2725)  TC AVFLAGA;
 *             (2726)  where AVFLAGA returns: EXTEND, DCA ATIGINC, TC
 *                     P35/P75A: DXCH KT, TC P20FLGON;
 *   P75       (2731)  TC AVFLAGP;
 *             (2732)  where AVFLAGP returns: EXTEND, DCA PTIGINC, on into
 *                     P35/P75A;
 *             (2736)  where P20FLGON returns: TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 35):
 *             (2737)  CALL SELECTMU;
 *   P35/P75B  (2741)  RTB LOADTIME;
 *             (2743)  where LOADTIME returns: STORE TSTRT, DAD KT, STORE
 *                     TIG, STORE INTIME, STCALL TDEC1 PRECSET;
 *             (2752)  where PRECSET returns: CALL S34/35.1;
 *             (2754)  where S34/35.1 returns: CALL S34/35.2;
 *             (2756)  where S34/35.2 returns: CALL S34/35.5;
 *             (2760)  where S34/35.5 returns: CALL VN1645;
 *             (2762)  where VN1645 returns: GOTO P35/P75B. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_P35_B35(z) ((uint16_t)(035u*1024u+((z)-02000u)))
#define LM_P35_INTERPRETIVE_ROUTINES 8u
#define LM_P35_BASIC_ROUTINES 5u
extern const agc_routine_table lm_p35_interpretive_routines;
extern const agc_routine_table lm_p35_basic_routines;
#endif
