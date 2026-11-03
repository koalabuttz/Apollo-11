/* SPDX-License-Identifier: MIT */
/* P32-P35_P72-P75.agc (Luminary099 pp. 632-641): CSI/A, the CSI
 * computation (bank 34), from its start to SCNDSOL. P32's CSI/A finds the
 * delta V of the CSI burn: a first guess, then (from NTP/2 on) the CDH and
 * TPI it leads to, and a Newton iteration on the guess.
 *
 * The listing's own words for its flags (p. 632): S32.1F1, "DVT1 HAS
 * EXCEEDED MAX INDICATOR"; S32.1F2, "FIRST PASS FOR NEWTON ITERATION
 * INDICATOR"; S32.1F3A and S32.1F3B, "00=1ST 2 PASSES 2ND CYCLE, 01=FIRST
 * CYCLE" and "10=2ND CYCLE, 11=50 FPS STAGE 2ND CYCLE".
 *
 * All interpretive: CSI_A_CODE (CSI/A to CIRCL) and NTP_2_CODE (NTP/2
 * to SCNDSOL) hold the words, which the readable
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

/* NTP/2 to SCNDSOL (pp. 635-641): the rest of CSI/A. */
static const word ntp_2_words[] = {
    /* NTP/2: TCDH, TCSI plus NN half periods less the time to the apsis
     * (the push-down list's), and TPI after it, else SCNDSOL with X2 5.
     * Both vehicles to TCDH (INTINT2C: the active vehicle's state after
     * CSI, VACT4 and RACT1, to RACT2 and VACT2; the passive's, RPASS1 and
     * VPASS1, to RPASS2 and VPASS2); CDHMVR, the CDH burn; its orbit's
     * pericenter (PERIAPO1, SHIFTR1) to POSTCDH. */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(NN),
    ADDRESS_PLUS_ONE(HAFPA1),
    ORDERS(SL, DSU),
    SL_COUNT(14),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TCSI),
    STORE(TCDH),
    ORDERS(BDSU, AXT_2),
    ADDRESS_PLUS_ONE(TTPI),
    NUMBER(5),
    ORDERS(BMN, SETPD),
    ADDRESS(SCNDSOL),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(VACT4),
    ADDRESS_PLUS_ONE(RACT1),
    ORDER(CALL),
    ADDRESS(INTINT2C),
    STOVL(RACT2),
    ADDRESS_PLUS_ONE(VATT),
    STOVL(VACT2),
    ADDRESS_PLUS_ONE(VPASS1),
    ORDERS(SETPD, PDVL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RPASS1),
    ORDER(CALL),
    ADDRESS(INTINT2C),
    STOVL(RPASS2),
    ADDRESS_PLUS_ONE(VATT),
    STCALL(VPASS2),
    ADDRESS(CDHMVR),
    ORDERS(VLOAD, SETPD),
    ADDRESS_PLUS_ONE(RACT2),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(VACT3),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    STOVL(POSTCDH),
    ADDRESS_PLUS_ONE(VACT3),
    /* Both vehicles on to TPI (INTINT, ACTIVE: RACT3, VACT3), and the line
     * of sight there the elevation angle ELEV above the horizontal: U,
     * (COSLOS)(UH3) plus (SINLOS)(URA3). The passive vehicle's on it at a
     * range K, a root of K**2 + 2(U . RA3)K + RA3.RA3 - RP3.RP3: TEMP2, the
     * discriminant ("TEMP1**2 + RA3.RA3 + RP3.RP3", with RA3.RA3
     * complemented), positive goes on at K1ORK2. Else, at the first pass
     * (LOOPCT 1) ALMXITA's alarm (X2 1); later, half the step back
     * (DELDV), from DVPREV, and round again at CSI/B1. */
    ORDERS(SETPD, PDVL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RACT2),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TCDH),
    ADDRESS_PLUS_ONE(TTPI),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TWOPI),
    ORDER(CALL),
    ADDRESS(INTINT),
    ORDER(CALL),
    ADDRESS(ACTIVE),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(SETPD, SIN),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(RACT3),
    STORE_D(0),        /* "URA3 AT 00D" */
    ORDERS(PDVL, VXV), /* PL14D, PL08D */
    ADDRESS_PLUS_ONE(UP1),
    ORDER(UNIT),
    ORDERS(PDDL, COS), /* "UNIT(URA3 X UVA3 X URA3) = UH3", PL14D */
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(VXSC, STADR), /* "(COSLOS)(UH3)", PL08D */
    AFTER_STADR(STORE_D(18)),
    ORDERS(DLOAD, VXSC), /* "(SINLOS)(URA3) = U", PL00D */
    ORDERS(VAD, VSL1),
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(PUSH, DOT),       /* PL06D */
    ADDRESS_PLUS_ONE(RACT3), /* "(U . RA3) = TEMP1" */
    ORDERS(SL1, PUSH),       /* PL08D */
    ORDERS(DSQ, TLOAD),      /* "TEMP1**2" */
    ADDRESS_PLUS_ONE(MPAC),
    ORDERS(PDVL, DOT), /* PL11D */
    ADDRESS_PLUS_ONE(RACT3),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(TLOAD, DCOMP), /* "RA3 . RA3" */
    ADDRESS_PLUS_ONE(MPAC),
    ORDERS(PDVL, DOT), /* "RP3 . RP3", PL14D */
    ADDRESS_PLUS_ONE(RPASS3),
    ADDRESS_PLUS_ONE(RPASS3), /* PL11D */
    ORDERS(TAD, TAD),         /* TEMP2, PL08D */
    ORDERS(BPL, DLOAD),
    ADDRESS(K1ORK2),
    ADDRESS_PLUS_ONE(LOOPCT),
    ORDERS(DSU, AXT_2),
    ADDRESS_PLUS_ONE(1DPB28),
    NUMBER(1),
    ORDER(BZE),
    ADDRESS(ALMXITA),
    ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(DELDV),
    STORE(DELDV),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(DVPREV),
    STCALL(DELVCSI),
    ADDRESS(CSI_B1),
    /* K1ORK2: "TEMP3 = TEMP2**.5"; K2, -TEMP1-TEMP3, at 10D; K1,
     * -TEMP1+TEMP3, at 12D; K the smaller in size (K2. with K2). */
    ORDERS(SQRT, PUSH), /* PL10D */
    ORDERS(DCOMP, DSU),
    ADDRESS_PLUS_ONE_D(6),    /* "-TEMP1-TEMP3 = K2 AT 10D" */
    STODL_D(10),              /* PL08D */
    ORDERS(DSU, STADR),       /* PL06D */
    AFTER_STADR(STORE_D(12)), /* "-TEMP1+TEMP3 = K1 AT 12D" */
    ORDER(ABS),
    STODL_D(14),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE_D(14),
    ORDERS(BMN, DLOAD),
    ADDRESS(K2_),
    ADDRESS_PLUS_ONE_D(12),
    STORE_D(10), /* "K = K1" */
    /* K2.: the line of sight's end, V = RA3 + KU, as a unit vector; GAMMA,
     * the angle from the passive vehicle's position to it in the passive
     * orbit's plane, its sign from TEMP, "(URP3 X V).(UVP3 X URP3)". */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(VXSC, VSL1),
    ORDERS(VAD, UNIT), /* "V = RA3 + KU UNIT" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(RPASS3), /* PL06D */
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(VPASS3), /* PL12D */
    ORDERS(VXV, PDVL),        /* "UVP3 X URP3", PL18D */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(STADR),             /* PL12D */
    AFTER_STADR(STOVL_D(12)), /* "(URP3 X V).(UVP3 X URP3)=TEMP", PL06D */
    ORDERS(DOT, SL1),         /* PL00D */
    ORDERS(ARCCOS, SIGN),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(SR1, PUSH), /* "GAMMA = SIGN(TEMP)ARCOS(UNITV.URP3)", PL02D */
    /* The first pass of a cycle (S32.1F2) at FRSTPAS. Else the slope,
     * "(GAM-GAMPREV)/(DV-DVPREV)", at 04D; THRDCHK unless in the second
     * cycle's 50 FPS stage (S32.1F3A and S32.1F3B set). In that stage,
     * with GAMMA-GAMPREV and GAMPREV of one sign FIFTYFPS again; else
     * DELDV back to INITST's 10 FPS, with DELDV's sign, the second cycle
     * (S32.1F3A set, S32.1F3B clear), and on into FRSTPAS. */
    ORDERS(BON, DLOAD),
    SWITCH(BON, S32_1F2),
    ADDRESS(FRSTPAS),
    ADDRESS_PLUS_ONE_D(0), /* "NOT THE FIRST PASS OF A CYCLE" */
    ORDERS(DSU, PDDL),     /* "GAMMA-GAMPREV", PL04D */
    ADDRESS_PLUS_ONE(GAMPREV),
    ADDRESS_PLUS_ONE(DELVCSI),
    ORDERS(DSU, NORM),
    ADDRESS_PLUS_ONE(DVPREV),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BDDV, PDDL),    /* "(GAM-GAMPREV)/(DV-DVPREV)", PL06D */
    ADDRESS_PLUS_ONE_D(2), /* "= SLOPE" */
    ADDRESS_PLUS_ONE(DELVCSI),
    STORE(DVPREV),
    ORDERS(BOFF, BOFF),
    SWITCH(BOF, S32_1F3A),
    ADDRESS(THRDCHK),
    SWITCH(BOF, S32_1F3B),
    ADDRESS(THRDCHK),
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE(GAMPREV),
    ORDERS(BPL, DLOAD),
    ADDRESS(FIFTYFPS),
    ADDRESS_PLUS_ONE(INITST),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(DELDV),
    STORE(DELDV),
    ORDERS(SET, CLEAR),
    SWITCH(SET, S32_1F3A),
    SWITCH(CLEAR, S32_1F3B),
    /* FRSTPAS: GAMMA to GAMPREV, DELVCSI to DVPREV, S32.1F2 clear, and
     * DELVCSI less DELDV, round again at CSI/B1. */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    STODL(GAMPREV),
    ADDRESS_PLUS_ONE(DELVCSI),
    STORE(DVPREV),
    ORDERS(DSU, CLEAR),
    ADDRESS_PLUS_ONE(DELDV),
    SWITCH(CLEAR, S32_1F2),
    STCALL(DELVCSI),
    ADDRESS(CSI_B1),
    /* THRDCHK: with S32.1F3A or S32.1F3B set, NEWTN; with both clear
     * (the second cycle's first two passes), on into FIFTYFPS. */
    ORDERS(BON, BON),
    SWITCH(BON, S32_1F3A),
    ADDRESS(NEWTN),
    SWITCH(BON, S32_1F3B),
    ADDRESS(NEWTN),
    /* FIFTYFPS: the step 50 FPS (FIFPSDP), its sign from the slope's and
     * GAMPREV's; DELVCSI less it; the 50 FPS stage (S32.1F3B and S32.1F3A
     * set); GAMMA to GAMPREV, and on at CSI/B2. */
    ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(FIFPSDP),
    ADDRESS_PLUS_ONE_D(4),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(GAMPREV),
    STORE(DELDV),
    ORDERS(DCOMP, DAD),
    ADDRESS_PLUS_ONE(DELVCSI),
    STODL(DELVCSI),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SET, SET),
    SWITCH(SET, S32_1F3B),
    SWITCH(SET, S32_1F3A),
    STCALL(GAMPREV),
    ADDRESS(CSI_B2),
    /* NEWTN: the Newton step, GAMMA over the slope (normalized, X2's
     * shift taken back by SR* 0,1), to DELDV; GAMMA to GAMPREV. Within
     * EPSILN1 (".1 FPS") CSI/SOL; else no more than DELMAX1 ("200 FPS"),
     * and CSISTEP: DELVCSI less DELDV, round again at CSI/B1. */
    ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE_D(4),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(BDDV, XSU_1),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(X2),
    ORDER(SR_STAR),
    SR_X1(0),
    STODL(DELDV),
    ADDRESS_PLUS_ONE_D(0),
    STORE(GAMPREV),
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(DELDV), /* PL08D */
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(EPSILN1),
    ORDERS(BMN, DLOAD),
    ADDRESS(CSI_SOL),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(DELMAX1),
    ADDRESS(CSISTEP),
    ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(DELMAX1),
    ADDRESS_PLUS_ONE(DELDV),
    STORE(DELDV),
    /* CSISTEP */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(DELVCSI),
    ADDRESS_PLUS_ONE(DELDV),
    STCALL(DELVCSI),
    ADDRESS(CSI_B1),
    /* CSI/SOL: the solution's checks, each failure to SCNDSOL with its
     * alarm number in X2: the perigee after CSI (2) and after CDH (3) at
     * least PMINE ("85 NM") about the earth or PMINM ("35000 FT") about
     * the moon (DSU* PMINE -2,1 by RTX1 in X1: "MUST BE 8 WORDS BEFORE
     * PMINM"); CSI to CDH (4) and CDH to TPI (5) at least TMIN ("10 MIN"),
     * T1TOT2 and T2TOT3. All met, on at P32/P72C. */
    ORDERS(DLOAD, AXT_2),
    ADDRESS_PLUS_ONE(POSTCSI),
    NUMBER(2),
    ORDER(LXA_1),
    ADDRESS(RTX1),
    ORDERS(DSU_STAR, BMN),
    ADDRESS_PLUS_ONE_AT_X1(PMINE, -2),
    ADDRESS(SCNDSOL),
    ORDERS(AXT_2, DLOAD),
    NUMBER(3),
    ADDRESS_PLUS_ONE(POSTCDH),
    ORDERS(DSU_STAR, BMN),
    ADDRESS_PLUS_ONE_AT_X1(PMINE, -2),
    ADDRESS(SCNDSOL),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TCDH),
    ADDRESS_PLUS_ONE(TCSI),
    STORE(T1TOT2),
    ORDERS(AXT_2, DSU),
    NUMBER(4),
    ADDRESS_PLUS_ONE(TMIN),
    ORDERS(BMN, AXT_2),
    ADDRESS(SCNDSOL),
    NUMBER(5),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TTPI),
    ADDRESS_PLUS_ONE(TCDH),
    STORE(T2TOT3),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(TMIN),
    ADDRESS(P32_P72C),
    /* SCNDSOL: in the second cycle (S32.1F3A set, or S32.1F3B clear), the
     * alarm at ALMXIT. Else X2 to CSIALRM, and the second cycle from
     * CSI/B: LOOPCT zero, S32.1F1 clear, S32.1F2 set, S32.1F3A and
     * S32.1F3B clear. */
    ORDERS(BON, BOFF),
    SWITCH(BON, S32_1F3A),
    ADDRESS(ALMXIT),
    SWITCH(BOF, S32_1F3B),
    ADDRESS(ALMXIT),
    ORDERS(SXA_2, DLOAD),
    ADDRESS(CSIALRM),
    ADDRESS_PLUS_ONE(P30ZERO),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, S32_1F1),
    SWITCH(SET, S32_1F2),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, S32_1F3A),
    SWITCH(CLEAR, S32_1F3B),
    STCALL(LOOPCT),
    ADDRESS(CSI_B),
};

const struct interpretive_code CSI_A_CODE = {034u, I_CSI_A % 02000u + 02000u, COUNT(csi_a_words),
                                             csi_a_words};

const struct interpretive_code NTP_2_CODE = {034u, I_NTP_2 % 02000u + 02000u, COUNT(ntp_2_words),
                                             ntp_2_words};

#endif
