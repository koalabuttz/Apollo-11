/* SPDX-License-Identifier: MIT */
/* P32-P35_P72-P75.agc (Luminary099 pp. 632-635): CSI/A, the CSI
 * computation (bank 34), from its start to CIRCL. P32's CSI/A finds the
 * delta V of the CSI burn: a first guess, then (from NTP/2 on, which is
 * still the faithful port's) the CDH and TPI it leads to, and a Newton
 * iteration on the guess.
 *
 * The listing's own words for its flags (p. 632): S32.1F1, "DVT1 HAS
 * EXCEEDED MAX INDICATOR"; S32.1F2, "FIRST PASS FOR NEWTON ITERATION
 * INDICATOR"; S32.1F3A and S32.1F3B, "00=1ST 2 PASSES 2ND CYCLE, 01=FIRST
 * CYCLE" and "10=2ND CYCLE, 11=50 FPS STAGE 2ND CYCLE".
 *
 * All interpretive: CSI_A_CODE holds the words, which the readable
 * interpreter runs. A comment names each stretch the way the listing's
 * comments do; "PL02D" is the push-down list's pointer after the order. */
#include "p32_p35.h"
#include "../agc/names.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word csi_a_words[] = {
    /* CSI/A: "INITIALIZE INDICATORS": S32.1F1 clear, S32.1F2 set (the
     * Newton iteration's first pass), the first cycle (S32.1F3A clear,
     * S32.1F3B set); LOOPCT and CSIALRM zero. */
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, S32_1F1),
    SWITCH(SET, S32_1F2),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, S32_1F3A),
    SWITCH(SET, S32_1F3B),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(P30ZERO),
    STORE(LOOPCT),
    STORE(CSIALRM),
    /* CSI/B: the first guess. The speed at RA1 of an orbit from RA1 to
     * RP3, the root of RTMU over (1 + RA1/RP3) RA1, each normalized (NORM
     * leaves its shift in X1 or X2, which the SR* and SL* take back off);
     * less VA1 . UH1, the active vehicle's speed along the horizontal UH1
     * (UP1 x RA1, unit), is DELVCSI. DELDV, the step, "10 FPS". */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(ABVAL, PUSH), /* RA1, PL02D */
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(PDVL, ABVAL), /* PL04D */
    ADDRESS_PLUS_ONE(RPASS3),
    ORDERS(NORM, BDDV), /* RA1/RP3, PL02D */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(XSU_2, SR_STAR),
    ADDRESS(X1),
    SR_X2(1),
    ORDERS(DAD, DMP), /* (1+(RA1/RP3))RA1, PL00D */
    ADDRESS_PLUS_ONE(1DPB2),
    ORDERS(NORM, PDDL), /* PL02D */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(RTMU),
    ORDERS(SR1, DDV), /* PL00D */
    ORDERS(SL_STAR, SQRT),
    SL_X1(-7),
    ORDERS(PDVL, UNIT), /* PL02D */
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(PDVL, VXV),
    ADDRESS_PLUS_ONE(UP1),
    ORDER(UNIT),      /* "UNIT(URP1 X UVP1 X URA1) = UH1" */
    ORDERS(DOT, SL1), /* VA1 . UH1 */
    ADDRESS_PLUS_ONE(VACT1),
    ORDERS(BDSU, STADR), /* PL00D */
    AFTER_STADR(STODL(DELVCSI)),
    ADDRESS_PLUS_ONE(INITST),
    STORE(DELDV),
    /* CSI/B1: "IF LOOPCT = 16" (LOOPMX) to SCNDSOL with X2 6; LOOPCT one
     * more. */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(LOOPCT),
    ADDRESS_PLUS_ONE(1DPB28),
    STORE(LOOPCT),
    ORDERS(DSU, AXT_2),
    ADDRESS_PLUS_ONE(LOOPMX),
    NUMBER(6),
    ORDER(BPL),
    ADDRESS(SCNDSOL),
    /* CSI/B2: DELVCSI within DVMAX1 ("MAXIMUM DV1 = 1000 FPS"), on at
     * CSI/B23. Else X2 7, and to SCNDSOL when S32.1F1 says it was over the
     * limit before, or in the second cycle's 50 FPS stage (S32.1F3A and
     * S32.1F3B both set). */
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(DELVCSI),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(DVMAX1),
    ADDRESS(CSI_B23),
    ORDERS(AXT_2, BON),
    NUMBER(7),
    SWITCH(BON, S32_1F1),
    ADDRESS(SCNDSOL),
    ORDERS(BOFF, BON),
    SWITCH(BOF, S32_1F3A),
    ADDRESS(CSI_B22), /* "FLAG 3 NEQ 3" */
    SWITCH(BON, S32_1F3B),
    ADDRESS(SCNDSOL),
    /* CSI/B22: S32.1F1 set, and DELVCSI DVMAX2 (989 FPS) with its sign. */
    ORDERS(SET, DLOAD),
    SWITCH(SET, S32_1F1),
    ADDRESS_PLUS_ONE(DVMAX2),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(DELVCSI),
    STORE(DELVCSI),
    /* CSI/B23: DELVEET1, DELVCSI along UH1; the velocity after it, VA1
     * plus DELVEET1 (BOV clears the overflow indicator). */
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(UP1),
    ORDERS(VXV, UNIT), /* "UNIT (URP1 X UVP1 X URA1) = UH1" */
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(DELVCSI),
    STORE(DELVEET1),
    ORDERS(VAD, BOV),
    ADDRESS_PLUS_ONE(VACT1),
    ADDRESS(CSI_B23D),
    /* CSI/B23D: to VACT4, and shifted (VECSHIFT) for TIMETHET: the time
     * round the orbit (SN359+ and CS359+, the sine and cosine of an angle
     * a hair short of 360 degrees), half of it to HAFPA1; PERIAPO's
     * pericenter (SHIFTR1) to POSTCSI. With CENTANG zero, or a circular
     * orbit (ECC under ONETHTH), on at CIRCL. */
    STCALL(VACT4),
    ADDRESS(VECSHIFT),
    STOVL(VVEC),
    ORDER(SET),
    SWITCH(SET, RVSW),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(SN359P),
    STCALL(SNTH), /* "ALSO CSTH" */
    ADDRESS(TIMETHET),
    ORDERS(SR1, LXA_1),
    ADDRESS(RTX1),
    STCALL(HAFPA1),
    ADDRESS(PERIAPO),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    STODL(POSTCSI),
    ADDRESS_PLUS_ONE(CENTANG),
    ORDERS(BZE, GOTO),
    ADDRESS_AT(CSI_B23D, 024u), /* "+2": the DLOAD below */
    ADDRESS(CIRCL),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ECC),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(ONETHTH),
    ADDRESS(CIRCL),
    /* The radial speed at R1 over R1 within NICKELDP ("7 FPS"), CIRCL;
     * else from P and R1 (RTSR1/MU) the time to the apsis ahead, to NTP/2
     * by CSI/B3. */
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(R1),
    ADDRESS(SHIFTR1),
    ORDERS(SETPD, NORM),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDVL, DOT), /* PL04D */
    ADDRESS_PLUS_ONE(RACT1),
    ADDRESS_PLUS_ONE(VACT4),
    ORDERS(ABS, DDV),
    ADDRESS_PLUS_ONE_D(2), /* "(/RDOTV/)/R1" */
    ORDERS(SL_STAR, DSU),
    SL_X1(0),
    ADDRESS_PLUS_ONE(NICKELDP),
    ORDERS(BMN, DLOAD),
    ADDRESS(CIRCL),
    ADDRESS_PLUS_ONE(P),
    ORDERS(SL2, DSU),
    ADDRESS_PLUS_ONE(1DPB2),
    STODL_D(14),
    ADDRESS_PLUS_ONE(RTSR1_MU),
    ORDERS(SR1, DDV), /* "(1/ROOTMU)/R1", PL02D */
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(P),
    ADDRESS_PLUS_ONE(R1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDERS(SL4, SL1),
    ORDERS(SQRT, DMP),
    ORDERS(BOFF, SL3),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(CSI_B3),
    /* CSI/B3: RDOTV, the radial speed after CSI; the angle to the apsis
     * into SNTH (and CSTH), TIMETHET's time to it from the state after CSI
     * (VCOMP'd for one going down), and from it (or HAFPA1 less it, going
     * down) to NTP/2. */
    ORDERS(PDVL, DOT),
    ADDRESS_PLUS_ONE(RACT1),
    ADDRESS_PLUS_ONE(VACT4),
    STORE(RDOTV),
    ORDER(ABS),
    ORDERS(NORM, DMP),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(XSU_1, SL_STAR),
    ADDRESS(X2),
    SL_X1(3),
    STODL_D(12),
    ADDRESS_PLUS_ONE(P30ZERO),
    STORE_D(16),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE_D(12),
    STOVL(SNTH),
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(PDVL, SIGN),
    ADDRESS_PLUS_ONE(VACT4),
    ADDRESS_PLUS_ONE(RDOTV),
    ORDERS(VCOMP, CALL),
    ADDRESS(VECSHIFT),
    STOVL(VVEC),
    ORDER(SET),
    SWITCH(SET, RVSW),
    STCALL(RVEC),
    ADDRESS(TIMETHET),
    ORDERS(PDDL, BPL),
    ADDRESS_PLUS_ONE(RDOTV),
    ADDRESS(NTP_2),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(HAFPA1),
    ORDERS(PUSH, GOTO),
    ADDRESS(NTP_2),
    /* CIRCL: no time to an apsis, zero on the push-down list. */
    ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(P30ZERO),
    ORDER(PUSH),
};

const struct interpretive_code CSI_A_CODE = {034u, I_CSI_A % 02000u + 02000u, COUNT(csi_a_words),
                                             csi_a_words};

#endif
