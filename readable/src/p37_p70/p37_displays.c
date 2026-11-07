/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 1 (Comanche055 pp. 895-899, bank 36): the
 * "RETURN TO EARTH DISPLAY SUBROUTINE" RTEVN; the "PRECISION DISPLAY,
 * TARGETING COMPUTATION AND RTE END PROCESSING" (P37G to P37W); and the
 * display subroutines P37GFRB1, P370GOFR and P370GOF.
 *
 * RTEVN shows a trajectory, the conic or the precision one: the landing
 * site (V06N61), the time to entry (V06N39), the speed and angle at entry
 * (V06N60) and the burn (V06N81). A recycle at any of them starts P37
 * again. P37N asks for the engine (V04N06, "SPS ASSUMED"), works out the
 * burn's length from the rocket equation and moves the time of ignition
 * earlier by part of it (V06N33, the "BIASED TIG"); P37W hands P40 or P41
 * their target and shows it (VN1645). */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC to an erasable word that holds a return: the AGC runs the word as an
 * instruction, a TC when the program has saved an address there. Any
 * other word (another instruction, or a register's address) is not
 * modelled. */
static void tc_return_in(unsigned address)
{
    word to = fetch(address);

    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_through(address);
}

/* INDEX address; TCF 0 +words: on at the return saved in an erasable
 * word, the given number of words past it. The sum must be a TCF into
 * fixed memory; any other is not modelled. */
static void tcf_past_return_in(unsigned address, word words)
{
    word to = indexed_instruction((word)(010000u + words), fetch(address));

    if (OPERATION_CODE(to) != 1u || ADDRESS_FIELD(to) < 02000u) not_modelled();
    continue_at(ADDRESS_FIELD(to));
}

/* ---- RTEVN -- pp. 895-896 ---- */

/* RTEVN: its return in VNSTORE (STQ), then RTEDISP, the "DISPLAY
 * PREPARATION", and out of the interpreter. */
static const word rtevn_words[] = {
    /* RTEVN */ ORDERS(STQ, CALL),
    ADDRESS(VNSTORE),
    ADDRESS(RTEDISP), /* "DISPLAY PREPARATION" */
    ORDER(EXIT),
};

const struct interpretive_code RTEVN_CODE = {036u, RTEVN, COUNT(rtevn_words), rtevn_words};

/* RTEVN +4, after the EXIT: "LATITUDE,LONGITUDE,BLANK" "IN
 * LAT(SPL),LNG(SPL),-" (V06N61, GOFLASHR). */
void rtevn_after_exit(void)
{
    set_A(V6N61RTE);
    p370gofr(RTEVN + 5u);
}

/* RTEVN +6, GOFLASHR's immediate return: R3 blanked (FOUR, at 37BLANK +1)
 * and the job ends. (Its TCF +5 after, to RTEVN +015, is a key too.) */
void rtevn_blank_r3(void)
{
    set_A(FOUR);
    tc_at(RTEVN + 7u);
    tc_to(ADDRESS_OF(37BLANK) + 1u, BLANKET);
}

/* RTEVN +012, its proceed: "T21 HRS,MIN,SEC IN T3TOT4" (V06N39). */
void rtevn_v6n39(void)
{
    set_A(V6N39RTE);
    p370gof(RTEVN + 013u);
}

/* RTEVN +015, its proceed: "DISPLAY BLANK,V(T2),FPA2" "IN
 * -,VPRED,GAMMAEI" (V06N60, R1 blanked). */
void rtevn_v6n60(void)
{
    set_A(V6N60RTE);
    p37gfrb1(RTEVN + 016u);
}

/* RTEVN +020, its proceed: "DISPLAY DELTA V (LV) IN DELVLVC" (V06N81). */
void rtevn_v6n81(void)
{
    set_A(V6N81RTE);
    p370gof(RTEVN + 021u);
}

/* RTEVN +023, its proceed: into the interpreter (TCR INTPRET), */
void rtevn_return(void)
{
    tc_at(RTEVN + 023u);
    intpret();
}

/* and back through VNSTORE (GOTO).
 *
 * P37G: the precision trajectory shown (RTEVN), then out of the
 * interpreter to P37N. */
static const word rtevn_return_words[] = {
    /* RTEVN +024 */ ORDER(GOTO), ADDRESS(VNSTORE),
    /* P37G */ ORDER(CALL),       ADDRESS(RTEVN),   ORDER(EXIT),
};

const struct interpretive_code RTEVN_RETURN_CODE = {036u, RTEVN + 024u, COUNT(rtevn_return_words),
                                                    rtevn_return_words};

/* ---- P37N to P37T -- pp. 896-897 ---- */

/* P37N, after the EXIT: OPTION1 7 and OPTION2 1, the SPS ("PROPULSION
 * SYSTEM OPTION" "1=SPS, 2=RCS"), for the "DISPLAY RCS OR SPS OPTION SPS
 * ASSUMED" (V04N06). (A is a constant: neither TS skips.) */
void p37n_after_exit(void)
{
    set_A(SEVEN);
    ts(ADDRESS_OF(OPTION1));
    set_A(ONE);
    ts(ADDRESS_OF(OPTION2));
    p37n_option();
}

/* P37N +4: the display; a recycle shows it again. */
void p37n_option(void)
{
    set_A(V4N06RTE);
    p370gof(P37N + 5u);
}

/* P37N +7, its proceed: into the interpreter, */
void p37n_proceed(void)
{
    tc_at(P37N + 7u);
    intpret();
}

/* where the engine's mass flow and exhaust speed are found: the SPS's
 * EMDOT and VCSPS; for the RCS (OPTION2 2, the 1RTEB13 it is compared
 * with), MDOTRCS, doubled once for two jets (NJETSFLG) and twice for
 * four. The mass flow is normalized (its shift in X1) on the push-down
 * list. P37T: "DV/VC", into the exponential's polynomial after the EXIT. */
static const word p37n_words[] = {
    /* P37N +010 */ ORDERS(SETPD, SLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(OPTION2),
    ORDERS(DSU, BZE),
    ADDRESS_PLUS_ONE(1RTEB13),
    ADDRESS(P37Q),
    ORDERS(SLOAD, NORM), /* "SPS" */
    ADDRESS_PLUS_ONE(EMDOT),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, GOTO),
    ADDRESS_PLUS_ONE(VCSPS),
    ADDRESS(P37T),
    /* P37Q */ ORDERS(DLOAD, BON), /* "RCS" */
    ADDRESS_PLUS_ONE(MDOTRCS),
    SWITCH(BON, NJETSFLG),
    ADDRESS(P37R),
    ORDER(SL1),
    /* P37R */ ORDER(SL1),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(VCRCS),
    /* P37T */ ORDERS(PDDL, DDV), /* "DV/VC" */
    ADDRESS_PLUS_ONE(DV),
    ORDER(EXIT),
};

const struct interpretive_code P37N_CODE = {036u, P37N + 010u, COUNT(p37n_words), p37n_words};

/* P37T +3, after the EXIT: TC POLY (DEC 1, then three 2DECs), back at
 * P37T +013. */
void p37t_after_exit(void)
{
    tc_to(P37T + 3u, POLY);
}

/* P37T +013: into the interpreter, */
void p37t_after_poly(void)
{
    tc_at(P37T + 013u);
    intpret();
}

/* where the polynomial is "(1-E)**(-DV/VC)=A"; the burn's length from
 * the mass (WEIGHT/G) and the flow, "DTB=(M0/MDOT)A", shifted back by the
 * flow's normalization (X1, less 12); and ignition moved earlier by a
 * part of it (CSUBT): "TIG=T1-CT*DTB". */
static const word p37t_poly_words[] = {
    /* P37T +014 */ ORDERS(PUSH, SLOAD), /* "(1-E)**(-DV/VC)=A" */
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(DMP, DDV), /* "DTB=(M0/MDOT)A" */
    ORDERS(SL_STAR, DMP),
    SL_X1(-12),
    ADDRESS_PLUS_ONE(CSUBT),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(T1),
    STORE(TIG), /* "TIG=T1-CT*DTB" */
    ORDER(EXIT),
};

const struct interpretive_code P37T_POLY_CODE = {036u, P37T + 014u, COUNT(p37t_poly_words),
                                                 p37t_poly_words};

/* P37T +026, after the EXIT: "DISPLAY BIASED TIG" (V06N33); a recycle
 * shows it again. */
void p37t_tig(void)
{
    set_A(V6N33RTE);
    p370gof(P37T + 027u);
}

/* P37T +031, its proceed: VHFCNT and TRKMKCNT +0 (A from a CAF has no
 * overflow for the TSes to skip on), and into the interpreter, */
void p37t_proceed(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(VHFCNT));
    ts(ADDRESS_OF(TRKMKCNT));
    tc_at(P37T + 034u);
    intpret();
}

/* ---- The end: to P37W -- pp. 897-898 ---- */

/* where the state is integrated to the entry ("CONICALLY INTEGRATE FROM
 * R1,V1 OVER T12", RTENCK1). With the entry point K/ . UR2 within 22.5
 * degrees of "K/=-UR1(COS7.5)-UH(SIN7.5)", on to P37W; else TIMETHET
 * from R(T1)/, V2(T1)/ through 165 or 210 degrees (THETA165, THETA210:
 * which, by the side UH/ puts R(T2)/ on), its sine and cosine to SNTH and
 * CSTH, RVSW clear.
 *
 * P37W: the targets for P40 and P41 ("THRUST PROGRAM COMMUNICATION"):
 * XDELVFLG and NORMSW clear ("SET 0 FOR LAMBERT AIMPT", "SET 0 FOR NO
 * ROTATION"), FINALFLG set; RTARG the "CONICALLY INTEGRATED REENTRY
 * POSITION VECTOR", TPASS4 the "REENTRY TIME" (T after T1), DELVSIN the
 * burn (V2(T1)/ less V(T1)/); VN1645 shows it, and again. */
static const word p37t_tig_words[] = {
    /* P37T +035 */ ORDER(CALL),
    ADDRESS(RTENCK1), /* "CONICALLY INTEGRATE FROM R1,V1 OVER T12" */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R_T2_),
    ORDERS(PDVL, VXSC), /* "UR2" */
    ADDRESS_PLUS_ONE(UR1_),
    ADDRESS_PLUS_ONE(MCOS7_5),
    ORDERS(PDVL, VXSC), /* "-UR1(COS7.5)" */
    ADDRESS_PLUS_ONE(UH_),
    ADDRESS_PLUS_ONE(MSIN7_5),
    ORDERS(VAD, DOT), /* "K/=-UR1(COS7.5)-UH(SIN7.5)" */
    ORDERS(DAD, BMN),
    ADDRESS_PLUS_ONE(MCOS22_5),
    ADDRESS(P37W),
    ORDERS(VLOAD, DOT), /* "K/ . UR2 GR COS22.5" */
    ADDRESS_PLUS_ONE(UH_),
    ADDRESS_PLUS_ONE(R_T2_),
    ORDERS(BMN, DLOAD),
    ADDRESS(P37U),
    ADDRESS_PLUS_ONE(THETA165),
    ORDERS(PUSH, GOTO),
    ADDRESS(P37V),
    /* P37U */ ORDERS(DLOAD, PUSH),
    ADDRESS_PLUS_ONE(THETA210),
    /* P37V */ ORDER(SIN),
    STODL(SNTH),
    ORDERS(COS, CLEAR),
    SWITCH(CLEAR, RVSW),
    STOVL(CSTH),
    ADDRESS_PLUS_ONE(R_T1_),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(V2_T1_),
    STCALL(VVEC),
    ADDRESS(TIMETHET),
    /* P37W */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, XDELVFLG),
    SWITCH(CLEAR, NORMSW),
    ORDERS(SET, VLOAD),
    SWITCH(SET, FINALFLG),
    ORDER(STADR),
    AFTER_STADR(STODL(RTARG)),
    ADDRESS_PLUS_ONE(T),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(T1),
    STOVL(TPASS4),
    ADDRESS_PLUS_ONE(V2_T1_),
    ORDER(VSU),
    ADDRESS_PLUS_ONE(V_T1_),
    STCALL(DELVSIN),
    ADDRESS(VN1645),
    ORDER(GOTO),
    ADDRESS(P37W),
};

const struct interpretive_code P37T_TIG_CODE = {036u, P37T + 035u, COUNT(p37t_tig_words),
                                                p37t_tig_words};

/* ---- The display subroutines -- pp. 898-899 ---- */

/* P37GFRB1, the "SUBROUTINE TO GO TO GOFLASHR AND BLANK R1", called by
 * TCR at `at`: its return in SPRTEX (EXTEND; QXCH), and P370GOFR. */
void p37gfrb1(word at)
{
    tc_at(at);
    qxch(ADDRESS_OF(SPRTEX));
    p370gofr(ADDRESS_OF(P37GFRB1) + 2u);
}

/* 37BLANK, GOFLASHR's immediate return to P37GFRB1: R1 blanked (ONE,
 * BLANKET), */
void blank37(void)
{
    set_A(ONE);
    tc_to(ADDRESS_OF(37BLANK) + 1u, BLANKET);
}

/* and the job ends. */
void blank37_after_blanket(void)
{
    continue_at(ENDOFJOB);
}

/* 37BLANK +3: "RECYCLE", back to the word after P37GFRB1's TCR (TC
 * SPRTEX). */
void blank37_recycle(void)
{
    tc_at(ADDRESS_OF(37BLANK) + 3u);
    tc_return_in(ADDRESS_OF(SPRTEX));
}

/* 37BLANK +4: "PROCEED", to the word after that (P37PROC). */
void blank37_proceed(void)
{
    tcf_past_return_in(ADDRESS_OF(SPRTEX), 1u);
}

/* P370GOFR, the "SUBROUTINE TO GO TO GOFLASHR", called by TCR at `at`:
 * its return in RTENCKEX (EXTEND; QXCH), and the display flashed by
 * GOFLASHR (BANKCALL), the verb and noun in A. */
void p370gofr(word at)
{
    tc_at(at);
    qxch(ADDRESS_OF(RTENCKEX));
    tc_to(P370GOFR + 2u, BANKCALL); /* CADR GOFLASHR */
}

/* P370GOFR +4: "TERMINATE". */
void p370gofr_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* P370GOFR +5: "PROCEED", on at the fourth word after the TCR (TCF +3;
 * INDEX RTENCKEX; TCF 0 +4). */
void p370gofr_proceed(void)
{
    tcf_past_return_in(ADDRESS_OF(RTENCKEX), 4u);
}

/* P370GOFR +6: "RECYCLE", on at the third (TCF +4; INDEX RTENCKEX; TCF 0
 * +3). */
void p370gofr_recycle(void)
{
    tcf_past_return_in(ADDRESS_OF(RTENCKEX), 3u);
}

/* P370GOFR +7: "IMMEDIATE RETURN", to the word after the TCR (TC
 * RTENCKEX). */
void p370gofr_immediate(void)
{
    tc_at(P370GOFR + 7u);
    tc_return_in(ADDRESS_OF(RTENCKEX));
}

/* P370GOF, the "SUBROUTINE TO GO TO GOFLASH", called by TCR at `at`: its
 * return in SPRTEX (EXTEND; QXCH), and the display flashed by GOFLASH
 * (BANKCALL), the verb and noun in A. */
void p370gof(word at)
{
    tc_at(at);
    qxch(ADDRESS_OF(SPRTEX));
    tc_to(P370GOF + 2u, BANKCALL); /* CADR GOFLASH */
}

/* P370GOF +4: terminate. */
void p370gof_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* P370GOF +5: proceed, at the second word after the TCR (TCF +2;
 * P37PROC: INDEX SPRTEX; TCF 0 +1). */
void p370gof_proceed(void)
{
    tcf_past_return_in(ADDRESS_OF(SPRTEX), 1u);
}

/* P370GOF +6: recycle, the word after the TCR (TC SPRTEX). */
void p370gof_recycle(void)
{
    tc_at(P370GOF + 6u);
    tc_return_in(ADDRESS_OF(SPRTEX));
}

#endif
