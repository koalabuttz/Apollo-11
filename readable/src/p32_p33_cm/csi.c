/* SPDX-License-Identifier: MIT */
/* P32-P33_P72-P73.agc (Comanche055 pp. 664-674): CSI/A, the CSI
 * computation, from its start to SCNDSOL. P32's CSI/A finds the delta V of
 * the CSI burn: a first guess, then (from NTP/2 on) the CDH and TPI it
 * leads to, and a Newton iteration on the guess. The code matches the
 * LM's (src/p32_p35/csi.c) but for its constants (ZEROVEC and ZEROVECS
 * for the LM's P30ZERO, 1RTEB2 for 1DPB2 once, INITST1 for INITST in the
 * second cycle, 600SEC for TMIN, 2PISC doubled for TWOPI) and its SETLOCs,
 * which cut it into ten stretches in ten banks, joined by GOTOs and
 * STCALLs.
 *
 * The listing's own words for its flags (p. 664): S32.1F1, "DVT1 HAS
 * EXCEEDED MAX INDICATOR"; S32.1F2, "FIRST PASS FOR NEWTON ITERATION
 * INDICATR"; S32.1F3A and S32.1F3B, "00=1ST 2 PASSES 2ND CYCLE 01=FIRST
 * CYCLE" and "10=2ND CYCLE, 11=50 FPS STAGE 2ND CYCLE".
 *
 * All interpretive: CSI_A_CODE (CSI/A to CSI/B1, bank 6), CSI_B2_CODE
 * (CSI/B2 and CSI/B22, bank 12), CSI_B23_CODE (CSI/B23 to CSI/B3, bank
 * 22), CSINEXT_CODE (CSINEXT and CIRCL, bank 23), NTP_2_CODE (NTP/2, bank
 * 23), CSINEXT1_CODE (bank 11), CSINEXT2_CODE (CSINEXT2 and K10RK2, bank
 * 7), K2_CODE (K2. to FRSTPAS, bank 24), CSINEXT3_CODE (CSINEXT3 to
 * CSI/SOL, bank 10) and CSINEXT4_CODE (CSINEXT4 and SCNDSOL, bank 31) hold
 * the words, which the readable interpreter runs. A comment names each stretch the way the
 * listing's comments do; "PL02D" is the push-down list's pointer after the
 * order. */
#include "p32_p33_cm.h"
#include "../agc/names.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* CSI/A to CSI/B1 (bank 6, pp. 664-665). */
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
    ADDRESS_PLUS_ONE(ZEROVEC),
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
     * more. Else to bank 12's CSI/B2. */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(LOOPCT),
    ADDRESS_PLUS_ONE(1DPB28),
    STORE(LOOPCT),
    ORDERS(DSU, AXT_2),
    ADDRESS_PLUS_ONE(LOOPMX),
    NUMBER(6),
    ORDERS(BPL, GOTO),
    ADDRESS(SCNDSOL),
    ADDRESS(CSI_B2),
};

/* CSI/B2 and CSI/B22 (bank 12, p. 665). */
static const word csi_b2_words[] = {
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
    /* CSI/B22: S32.1F1 set, and DELVCSI DVMAX2 (989 FPS) with its sign;
     * to bank 22's CSI/B23. */
    ORDERS(SET, DLOAD),
    SWITCH(SET, S32_1F1),
    ADDRESS_PLUS_ONE(DVMAX2),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(DELVCSI),
    STCALL(DELVCSI),
    ADDRESS(CSI_B23),
};

/* CSI/B23 to CSI/B3 (bank 22, pp. 665-667). */
static const word csi_b23_words[] = {
    /* CSI/B23: DELVEET1, DELVCSI along UH1; the velocity after it, VA1
     * plus DELVEET1 (BOV clears the overflow indicator). */
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(UP1),
    ORDERS(VXV, UNIT), /* "UNIT(URP1 X UVP1 X URA1) = UH1" */
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
    ADDRESS_PLUS_ONE(1RTEB2), /* "1.B.2" */
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
     * into SNTH (and CSTH), and the state after CSI (VCOMP'd for one going
     * down) for TIMETHET, at bank 23's CSINEXT. */
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
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_D(16),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE_D(12),
    STOVL(SNTH), /* "ALSO STORES CSTH AND 0" */
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(PDVL, SIGN),
    ADDRESS_PLUS_ONE(VACT4),
    ADDRESS_PLUS_ONE(RDOTV),
    ORDERS(VCOMP, CALL),
    ADDRESS(VECSHIFT),
    STOVL(VVEC),
    ORDER(SETGO),
    SWITCH(SETGO, RVSW),
    ADDRESS(CSINEXT),
};

/* CSINEXT and CIRCL (bank 23, pp. 667-668). */
static const word csinext_words[] = {
    /* CSINEXT: TIMETHET's time to the apsis, and from it (or HAFPA1 less
     * it, going down) to NTP/2. */
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
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDER(PUSH),
};

/* NTP/2 (bank 23, p. 668): TCDH, TCSI plus NN half periods less the time
 * to the apsis (the push-down list's), and TPI after it, else SCNDSOL with
 * X2 5. Both vehicles to TCDH (INTINT2C: the active vehicle's state after
 * CSI, VACT4 and RACT1, to RACT2 and VACT2; the passive's, RPASS1 and
 * VPASS1, at bank 11's CSINEXT1). */
static const word ntp_2_words[] = {
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
    ORDER(GOTO),
    ADDRESS(CSINEXT1),
};

/* CSINEXT1 (bank 11, pp. 668-669): the passive vehicle to TCDH (RPASS2,
 * VPASS2); CDHMVR, the CDH burn; its orbit's pericenter (PERIAPO1,
 * SHIFTR1) to POSTCDH. Both vehicles on to TPI (INTINT, ACTIVE: RACT3,
 * VACT3), and the line of sight there the elevation angle ELEV above the
 * horizontal: U, (COSLOS)(UH3) plus (SINLOS)(URA3), at bank 7's
 * CSINEXT2. */
static const word csinext1_words[] = {
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
    /* to TPI: TTPI, and 2PISC doubled (SL2) */
    ORDERS(SETPD, PDVL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RACT2),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TCDH),
    ADDRESS_PLUS_ONE(TTPI),
    ORDERS(PDDL, SL2),
    ADDRESS_PLUS_ONE(2PISC),
    ORDERS(PUSH, CALL),
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
    ORDERS(PDVL, VXV), /* "PL14D,PL08D" */
    ADDRESS_PLUS_ONE(UP1),
    ORDER(UNIT),
    ORDERS(PDDL, COS), /* "UNIT(URA3XUVA3XURA3) = UH3", PL14D */
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(VXSC, STADR), /* "(COSLOS)(UH3)", PL08D */
    AFTER_STADR(STCALL_D(18)),
    ADDRESS(CSINEXT2),
};

/* CSINEXT2 and K10RK2 (bank 7, pp. 669-670). The passive vehicle is on
 * the line of sight at a range K, a root of K**2 + 2(U . RA3)K + RA3.RA3
 * - RP3.RP3: TEMP2, the discriminant ("TEMP1**2+RA3.RA3+RP3.RP3=TEMP2",
 * with RA3.RA3 complemented), positive goes on at K10RK2. Else, at the
 * first pass (LOOPCT 1) ALMXITA's alarm (X2 1); later, half the step back
 * (DELDV), from DVPREV, and round again at CSI/B1. */
static const word csinext2_words[] = {
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
    ORDERS(TLOAD, DCOMP), /* "RA3.RA3" */
    ADDRESS_PLUS_ONE(MPAC),
    ORDERS(PDVL, DOT), /* "RP3.RP3", PL14D */
    ADDRESS_PLUS_ONE(RPASS3),
    ADDRESS_PLUS_ONE(RPASS3), /* PL11D */
    ORDERS(TAD, TAD),         /* TEMP2, PL08D */
    ORDERS(BPL, DLOAD),
    ADDRESS(K10RK2),
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
    /* K10RK2: "TEMP3 = TEMP2**.5"; K2, -TEMP1-TEMP3, at 10D; K1,
     * -TEMP1+TEMP3, at 12D; K the smaller in size, at bank 24's K2. */
    ORDERS(SQRT, PUSH), /* PL10D */
    ORDERS(DCOMP, DSU),
    ADDRESS_PLUS_ONE_D(6),    /* "-TEMP1-TEMP3 =K2 AT 10D" */
    STODL_D(10),              /* PL08D */
    ORDERS(DSU, STADR),       /* PL06D */
    AFTER_STADR(STORE_D(12)), /* "-TEMP1+TEMP3 =K1 AT 12D" */
    ORDER(ABS),
    STODL_D(14),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE_D(14),
    ORDERS(BMN, DLOAD),
    ADDRESS(K2_),
    ADDRESS_PLUS_ONE_D(12),
    STCALL_D(10), /* "K EQUALS K1" */
    ADDRESS(K2_),
};

/* K2. to FRSTPAS (bank 24, pp. 670-671). */
static const word k2_words[] = {
    /* K2.: the line of sight's end, V = RA3 + KU, as a unit vector;
     * GAMMA, the angle from the passive vehicle's position to it in the
     * passive orbit's plane, its sign from TEMP. */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(VXSC, VSL1),
    ORDERS(VAD, UNIT), /* "V=RA3+KU  UNIT" */
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
    AFTER_STADR(STOVL_D(12)), /* "(URP3XV).(UVP3XURP3)=TEMP", PL06D */
    ORDERS(DOT, SL1),         /* PL00D */
    ORDERS(ARCCOS, SIGN),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(SR1, PUSH), /* "GAMMA=SIGN(TEMP)ARCOS(UNITV.URP3)", PL02D */
    /* The first pass of a cycle (S32.1F2) at FRSTPAS. Else the slope,
     * "(GAM-GAMPREV)/(DV-DVPREV)", at 04D; THRDCHK unless in the second
     * cycle's 50 FPS stage (S32.1F3A and S32.1F3B set). In that stage,
     * with GAMMA-GAMPREV and GAMPREV of one sign FIFTYFPS again; else
     * DELDV back to INITST1's 10 FPS, with DELDV's sign, the second cycle
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
    ADDRESS_PLUS_ONE(INITST1),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(DELDV),
    STORE(DELDV),
    ORDERS(SET, CLEAR),
    SWITCH(SET, S32_1F3A),
    SWITCH(CLEAR, S32_1F3B),
    /* FRSTPAS: GAMMA to GAMPREV, DELVCSI to DVPREV, and on at bank 10's
     * CSINEXT3. */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    STODL(GAMPREV),
    ADDRESS_PLUS_ONE(DELVCSI),
    STCALL(DVPREV),
    ADDRESS(CSINEXT3),
};

/* CSINEXT3 to CSI/SOL (bank 10, pp. 672-673). */
static const word csinext3_words[] = {
    /* CSINEXT3: S32.1F2 clear, and DELVCSI less DELDV, round again at
     * CSI/B1. */
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
     * set); GAMMA to GAMPREV, and on at bank 12's CSI/B2. */
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
     * alarm number in X2, from POSTCSI (2) with RTX1 in X1, at bank 31's
     * CSINEXT4. */
    ORDERS(DLOAD, AXT_2),
    ADDRESS_PLUS_ONE(POSTCSI),
    NUMBER(2),
    ORDERS(LXA_1, GOTO),
    ADDRESS(RTX1),
    ADDRESS(CSINEXT4),
};

/* CSINEXT4 and SCNDSOL (bank 31, pp. 673-674). */
static const word csinext4_words[] = {
    /* CSINEXT4: the perigee after CSI (2) and after CDH (3) at least PMINE
     * ("84 NM.") about the earth or PMINM ("35000 FT") about the moon
     * (DSU* PMINE -2,1 by RTX1 in X1: "MUST BE 8 WORDS BEFORE PMINM"); CSI
     * to CDH (4) and CDH to TPI (5) at least 600SEC ("10 MIN"), T1TOT2 and
     * T2TOT3. All met, on at bank 4's P32/P72C. */
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
    ADDRESS_PLUS_ONE(600SEC),
    ORDERS(BMN, AXT_2),
    ADDRESS(SCNDSOL),
    NUMBER(5),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TTPI),
    ADDRESS_PLUS_ONE(TCDH),
    STORE(T2TOT3),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(600SEC),
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
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, S32_1F1),
    SWITCH(SET, S32_1F2),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, S32_1F3A),
    SWITCH(CLEAR, S32_1F3B),
    STCALL(LOOPCT),
    ADDRESS(CSI_B),
};

const struct interpretive_code CSI_A_CODE = {06u, I_CSI_A % 02000u + 02000u, COUNT(csi_a_words),
                                             csi_a_words};
const struct interpretive_code CSI_B2_CODE = {012u, I_CSI_B2 % 02000u + 02000u, COUNT(csi_b2_words),
                                              csi_b2_words};
const struct interpretive_code CSI_B23_CODE = {022u, I_CSI_B23 % 02000u + 02000u,
                                               COUNT(csi_b23_words), csi_b23_words};
const struct interpretive_code CSINEXT_CODE = {023u, I_CSINEXT % 02000u + 02000u,
                                               COUNT(csinext_words), csinext_words};

const struct interpretive_code NTP_2_CODE = {023u, I_NTP_2 % 02000u + 02000u, COUNT(ntp_2_words),
                                             ntp_2_words};
const struct interpretive_code CSINEXT1_CODE = {011u, I_CSINEXT1 % 02000u + 02000u,
                                                COUNT(csinext1_words), csinext1_words};
const struct interpretive_code CSINEXT2_CODE = {07u, I_CSINEXT2 % 02000u + 02000u,
                                                COUNT(csinext2_words), csinext2_words};
const struct interpretive_code K2_CODE = {024u, I_K2_ % 02000u + 02000u, COUNT(k2_words), k2_words};
const struct interpretive_code CSINEXT3_CODE = {010u, I_CSINEXT3 % 02000u + 02000u,
                                                COUNT(csinext3_words), csinext3_words};
const struct interpretive_code CSINEXT4_CODE = {031u, I_CSINEXT4 % 02000u + 02000u,
                                                COUNT(csinext4_words), csinext4_words};

#endif
