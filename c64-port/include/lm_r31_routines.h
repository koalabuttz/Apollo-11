/* SPDX-License-Identifier: MIT */
#ifndef LM_R31_ROUTINES_H
#define LM_R31_ROUTINES_H
/* Luminary099 R31 (bank 40 03674-03712, bank 37 03113-03407) as routines of
 * the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md, docs/LM-R31.md). io
 * is a p66_bias_io.
 *
 * Basic (keyed by Z; bank 40 is FB 30 with SUPERBNK 4):
 *   R31CALL (40,3674) CAF PRIO3, TC FINDVAC (2CADR V83CALL);
 *   DSPDELAY (40,3700) TC FIXDELAY (DEC 100); (40,3702) BZF DSPDELAY while
 *          EXTVBACT BIT12 is clear, else CAF PRIO5, TC NOVAC (2CADR
 *          DISPN5X); (40,3712) TCF TASKOVER;
 *   DISPN5X (3113) CAF V16N54, TC BANKCALL (CADR GOMARKF); its returns
 *          (3116) and (3117) TC B5OFF, (3120) TCF DISPN5X;
 *   V83CALL (3121) and REV83 (3155): BZF to MUNG? or GETRVN when AVEGFLAG
 *          is set, to ONEBASE or R31SURF when SURFFLAG is set, else TC
 *          INTPRET;
 *   (3316) after the EXIT: BZF ENDEXT unless EXTVBACT BIT5, else set BIT12
 *          (ADS) and TCF REV83;
 *   GETRVN (3326) CA PRIO22, TC PRIOCHNG; (3330) TC INTPRET; (3344) CA
 *          PRIO3, TC PRIOCHNG; (3346) TC INTPRET; R31SURF (3370) TC
 *          INTPRET; MUNG? (3377) BZF GETRVN unless MUNFLAG, on into ONEBASE
 *          (3403) TC INTPRET.
 * Interpretive (keyed by LOC under BANKSET at NEWOPS in bank 37):
 *   (3132), (3166), (3371) and ONEBASE (3404) RTB LOADTIME;
 *   (3134) and (3373) STCALL TDEC1 LEMPREC; (3136) the LM base state
 *          (BASETHP, BASETHV), into DOCMBASE (3144) BASETIME, STCALL TDEC1
 *          CSMPREC; (3147) the CSM base state (BASEOTP, BASEOTV), EXIT;
 *   (3170) STCALL TDEC1 INTSTALL; (3172) RCV, VCV from the LM base state,
 *          MOONFLAG by MOONTHIS, SET INTYPFLG, STCALL TET INTEGRVS;
 *   OTHCONIC (3211) RONE, VONE, STCALL INTSTALL; (3217) SET INTYPFLG,
 *          DLOAD TAT, into OTHINT (3222) the same integration of the CSM
 *          base state;
 *   COMPDISP (3240) RTB NORMUNX1 (3246) RANGE (SL* 0,1), RRATE, UNIT PDVL
 *          UNITZ, CALL CDU*NBSM; (3263) the LOS angle through REFSMMAT (VXM,
 *          VPROJ, BVSU, UNIT, VXV, DOT, SIGN), SL2 ACOS (to ARCCOS);
 *          (3304) RTHETA, DPPOSMAX - RTHETA unless the BPL is taken, EXIT;
 *   GETRVN (3331) RN, VN, V(CSM), R(CSM), PIPTIME, EXIT; (3347) BOFF
 *          MUNFLAG GETRVN2, else the REFSMMAT rotations (VXM, and the
 *          second at 3355) and GOTO COMPDISP; GETRVN2 (3363) CALL INTSTALL; (3365) CLEAR INTYPFLG,
 *          GOTO OTHINT; (3375) GOTO OTHCONIC. */
#include "agc_transfer.h"
#include "p66_bias.h"
#define LM_R31_B37(z) ((uint16_t)(037u*1024u+((z)-02000u)))
#define LM_R31_B40(z) ((uint16_t)(040u*1024u+((z)-02000u)))
#define LM_R31_INTERPRETIVE_ROUTINES 24u
#define LM_R31_BASIC_ROUTINES 18u
extern const agc_routine_table lm_r31_interpretive_routines;
extern const agc_routine_table lm_r31_basic_routines;
#endif
