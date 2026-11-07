/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc (Comanche055 pp. 844-882, banks 25 and 26): the
 * entry guidance, P63 to P67. Every two seconds, through SERVICER's
 * CM/POSE, SCALEPOP gets the state (TARGETNG: the velocity, drag D, the
 * range to the target THETAH) and goes on at GOTOADDR, "ADDRESS SELECTED
 * BY SEQUENCER": the roll command equations of the present phase of
 * entry, INITROLL, HUNTEST, UPCONTRL, KEP2 or PREDICT3. Each ends in the g
 * limiter and the lateral logic (GLIMITER, LIMITL/D), which set ROLLC, the
 * "ROLL COMMAND", and the pass ends at OVERNOUT, with the display. In the
 * order of the listing:
 *
 *   STARTENT, STARTEN1, SCALEPOP, REFAZE10 (startent.c, pp. 844-847):
 *     the start, the target vector, and the two-second pass.
 *   TARGETNG, GETETA (targetng.c, pp. 848-852): the state and the range.
 *   INITROLL, HUNTEST (initroll.c, pp. 853-858): the initial roll and the
 *     hunt test.
 *   RANGER to GOTOUPSY (ranger.c, pp. 859-864): the range prediction, the
 *     hunt's step in the lift, its restart, and into the up control.
 *   UPCONTRL, DCONSTD (upcontrl.c, pp. 864-868): the up control, the
 *     constant drag and the down control.
 *   KEP, PREDICT3 (kep.c, pp. 869-871): the ballistic phase and the final
 *     phase's range prediction.
 *   HAVDNRNG to ENDEXIT (glimiter.c, pp. 871-873): the final phase's L/D,
 *     the g limiter, the lateral logic and the roll command.
 *   OVERNOUT to L357 (overnout.c, pp. 873-875): the end of each pass, and
 *     of the steering.
 *
 * The rest of the file is data, read from the rope: the "TABLE USED FOR
 * SUB-ORBITAL REFERENCE TRAJECTORY CONTROL." (VREFER, pp. 876-878: "13
 * POINTS ARE STORED AS THE INDEPENDENT" "VARIABLE AND THEN SIX 13 POINT
 * FUNCTIONS" "OF V") and the "REENTRY CONSTANTS." (pp. 879-882).
 *
 * ENTRY_LEXICON.agc (pp. 837-843) has no code: it is the guidance's
 * glossary, the names of the GSOP's flow charts with the program's
 * ("COMPUTER NAME") and their scaling ("MAXIMUM VALUE"). A few to read
 * the code by:
 *
 *   D           "TOTAL ACCELERATION", the drag, scaled 805 FPSS (25 g).
 *   V, RDOT     the speed and the "ALTITUDE RATE", scaled "2 VSAT".
 *   VSQUARE     "NORMALISED VEL. SQUARED"; LEQ, "EXCESS C.F. OVER
 *               GRAV=(VSQ-1)GS".
 *   THETAH      "DESIRED RANGE (RADIANS)", to the target.
 *   L/D         "DESIRED LIFT TO DRAG RATIO" "(VERTICAL PLANE)"; LAD,
 *               "MAX L/D (MIN ACTUAL VEHICLE L/D)"; LOD, "FINAL PHASE
 *               L/D".
 *   Q7          "MINIMUM DRAG FOR UPCONTROL"; V1, VL, "INITIAL VELOCITY
 *               FOR UPCONTROL" and "EXIT VELOCITY FOR UPCONTROL"; GAMMAL,
 *               "FLIGHT PATH ANGLE AT VL".
 *   LEWD        "UPCONTROL REFERENCE L/D", which HUNTEST hunts.
 *   FX to FX +5 the final phase's columns of the table: F3, AREF, RTOGO,
 *               RDOTREF, F2, F1.
 *   The switches in CM/FLAGS ("CM/FLAGS = STATE +6"): ENTRYDSP, GONEPAST,
 *   RELVELSW, EGSW, HIND, INRLSW, LATSW, .05GSW; and GONEBY.
 *
 * The faithful C is c64-port/src/cm_reentry.c.
 *
 * Each basic function runs from a word the program can resume at (an
 * entry, a return from a routine out of the file, or the word after an
 * EXIT) until its code leaves it: at a TC out of the file (Z the routine,
 * Q the return) or at a TC INTPRET (run on to NEWOPS). The interpretive
 * code is the listing's words, run by the readable interpreter. */
#ifndef READABLE_REENTRY_H
#define READABLE_REENTRY_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* STARTENT to REFAZE10 -- startent.c */
void startent_after_exit(void);     /* STARTENT +1 */
void refaze10(void);                /* REFAZE10 (SCALEPOP's EXIT) */
void refaze10_after_phaschng(void); /* REFAZE10 +2 */
extern const struct interpretive_code STARTENT_CODE, STARTENT_INTERPRETIVE_CODE, SCALEPOP_CODE,
    REFAZE10_CODE;

/* TARGETNG and GETETA -- targetng.c */
extern const struct interpretive_code TARGETNG_CODE, GETETA_CODE;

/* INITROLL and HUNTEST -- initroll.c */
extern const struct interpretive_code INITROLL_CODE, HUNTEST_CODE;

/* RANGER to GOTOUPSY -- ranger.c */
void sidetrak_after_exit(void);     /* SIDETRAK +1 */
void sidetrak_after_phaschng(void); /* SIDETRAK +7 */
void sidetrak_after_priochng(void); /* SIDETRAK +011 */
void pre_hunt(void);                /* PRE-HUNT */
extern const struct interpretive_code RANGER_CODE, SIDETRAK_CODE, PRE_HUNT_CODE, GOTOUPSY_CODE;

/* UPCONTRL to DOWNCNTL -- upcontrl.c */
extern const struct interpretive_code UPCONTRL_CODE, DCONSTD_CODE;

/* KEP to PREDICT3 -- kep.c */
void predict3_after_exit(void); /* PREDICT3 +5 */
extern const struct interpretive_code KEP_CODE, PREDICT3_CODE;

/* HAVDNRNG to ENDEXIT -- glimiter.c */
extern const struct interpretive_code HAVDNRNG_CODE;

/* OVERNOUT to L357 -- overnout.c */
void overnout(void);                /* OVERNOUT (ENDEXIT's EXIT) */
void nodisky(void);                 /* NODISKY */
void servnout(void);                /* SERVNOUT */
void steeroff_after_exit(void);     /* STEEROFF +1 */
void steeroff_after_novac(void);    /* STEEROFF +7 */
void steeroff_after_2phschng(void); /* STEEROFF +012 */
extern const struct interpretive_code STEEROFF_CODE, STEEROFF_INTERPRETIVE_CODE;

#endif

#endif
