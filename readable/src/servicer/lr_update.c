/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 884-891 (Luminary099 only, bank 33): the landing
 * radar's measurements worked into the state. UPDATCHK, which decides
 * whether there is anything to do; POSUPDAT, the altitude measurement
 * checked against the computed altitude and weighted into the position;
 * VMEASCHK and VELUPDAT, a velocity beam's measurement checked against the
 * computed velocity, and VUPDAT, which weights it into the velocity;
 * VALTCHK, which decides whether to read the velocity next; GNURVST,
 * which stores the new position or velocity; QUIKFAZ5, the restart point;
 * and HFAIL, VFAIL, NORLITE and NOLITE, which count bad measurements for
 * the tracker fail lamp. The faithful C is c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* INDEX FIXLOC is modelled with FIXLOC at one of the five VAC areas, as a
 * job's work area is. */
static bool fixloc_at_a_vac_area(void)
{
    unsigned n;

    for (n = 1u; n <= VAC_AREAS; ++n)
        if (FIXLOC == VACNUSE_ADDRESS(n) + 1u) return true;
    return false;
}

/* ---- UPDATCHK, POSUPDAT -- pp. 884-885 ---- */

/* UPDATCHK: "SEE IF LR UPDATE INHIBITED." (NOLRRBIT of FLGWRD11): "IT IS
 * -- NO LR UPDATE" (CONTSERV). Else with no RNGEDBIT, "NO ALT MEAS THIS
 * CYCLE -- CHECK FOR VEL" (VMEASCHK); else POSUPDAT. */
void updatchk(void)
{
    set_A(NOLRRBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        contserv();
        return;
    }
    set_A(RNGEDBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        vmeaschk();
        return;
    }
    posupdat();
}

/* POSUPDAT: "SET PUSHLIST TO ZERO", and to the interpreter. */
void posupdat(void)
{
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(PUSHLOC));
    tc_at(POSUPDAT + 2u);
    intpret();
}

/* The altitude beam in the stable member's frame ("HBEAM SM AT 2(2)"),
 * pushed; the velocity relative to the surface (V1S less DELVS) along it,
 * "SCALE TO RADAR COUNTS X 5" (RADSKAL): the Doppler correction. */
static const word posupdat_words[] = {
    /* POSUPDAT +3 */ ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(HBEAMNB),
    ADDRESS_PLUS_ONE(XNBPIP), /* "HBEAM SM AT 2(2)" */
    ORDERS(PDVL, VSL2),       /* "STORE HBEAM IN PD 0-5" */
    ADDRESS_PLUS_ONE(V1S),    /* "SCALE V AT 2(5) M/CS" */
    ORDERS(VAD, DOT),
    ADDRESS_PLUS_ONE(DELVS), /* "V RELATIVE TO SURFACE AT 2(5) M/CS" */
    ADDRESS_PLUS_ONE_D(0),   /* "V ALONG HBEAM AT 2(7) M/CS." */
    ORDERS(DMP, EXIT),
    ADDRESS_PLUS_ONE(RADSKAL), /* "SCALE TO RADAR COUNTS X 5" */
};

const struct interpretive_code POSUPDAT_CODE = {033u, POSUPDAT + 3u, COUNT(posupdat_words),
                                                posupdat_words};

/* After the EXIT: "TEST LR ALTITUDE SCALE FACTOR" (ALTSCBIT of FLGWRD12):
 * "RESCALE IF LOW SCALE" (SKALSKAL, by SHORTMP); ... */
void posupdat_after_exit(void)
{
    cs(ADDRESS_OF(FLGWRD12));
    mask(ALTSCBIT);
    if (A_is_zero()) { /* "BRANCH IF HIGH SCALE" */
        posupdat_after_shortmp();
        return;
    }
    ca(ADDRESS_OF(SKALSKAL));
    tc_to(POSUPDAT + 022u, SHORTMP);
}

/* ... and to the interpreter. */
void posupdat_after_shortmp(void)
{
    tc_at(POSUPDAT + 023u);
    intpret();
}

/* "CORRECT HMEAS FOR DOPPLER EFFECT", the slant range scaled (HSCAL) along
 * the beam, and its height along UNIT/R/ less HCALC: DELTAH, "DELTA H AT
 * 2(24) M". */
static const word posupdat_2_words[] = {
    /* POSUPDAT +24 */ ORDERS(DAD, SL), /* "CORRECT HMEAS FOR DOPPLER EFFECT" */
    ADDRESS_PLUS_ONE(HMEAS),
    SL_COUNT(7),
    ORDERS(DMP, VXSC),       /* "SLANT RANGE AT 2(21), PUSH UP FOR HBEAM" */
    ADDRESS_PLUS_ONE(HSCAL), /* "SLANT RANGE VECTOR AT 2(23) M" */
    ORDERS(DOT, DSU),
    ADDRESS_PLUS_ONE(UNIT_R_), /* "ALTITUDE AT 2(24) M" */
    ADDRESS_PLUS_ONE(HCALC),   /* "DELTA H AT 2(24) M" */
    STORE(DELTAH),
    ORDER(EXIT),
};

const struct interpretive_code POSUPDAT_2_CODE = {033u, POSUPDAT + 024u, COUNT(posupdat_2_words),
                                                  posupdat_2_words};

/* After it: "DO NOT PERFORM DATA REASONABLENESS TEST" "UNTIL AFTER HIGATE"
 * (PSTHIBIT): NOREASON. Else to the interpreter, for the test. */
void posupdat_after_second_exit(void)
{
    ca(ADDRESS_OF(FLGWRD11));
    mask(PSTHIBIT);
    if (A_is_zero()) {
        noreason();
        return;
    }
    tc_at(POSUPDAT + 042u);
    intpret();
}

/* "ABS(DELTAH) - DQFIX	50 FT NOM", scaled, less HCALC: "ABS(DELTAH) - (50
 * + HCALC/8) AT 2(21)". */
static const word posupdat_3_words[] = {
    /* POSUPDAT +43 */ ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(DELQFIX), /* "ABS(DELTAH) - DQFIX	50 FT NOM" */
    ORDERS(SL3, DSU),          /* "SCALE TO 2(21)" */
    ADDRESS_PLUS_ONE(HCALC),   /* "ABS(DELTAH) - (50 + HCALC/8) AT 2(21)" */
    ORDER(EXIT),
};

const struct interpretive_code POSUPDAT_3_CODE = {033u, POSUPDAT + 043u, COUNT(posupdat_3_words),
                                                  posupdat_3_words};

/* After it: LRLCTR counted, and BRANCH on the difference in MPAC: ... */
void posupdat_after_third_exit(void)
{
    incr(ADDRESS_OF(LRLCTR));
    tc_to(POSUPDAT + 051u, BRANCH);
}

/* ... positive or +0, "DELTA H TOO LARGE" (HFAIL); ... */
void posupdat_after_branch(void)
{
    hfail();
}

void posupdat_after_branch_zero(void)
{
    hfail();
}

/* ... negative: "TURN OFF ALT FAIL LAMP" (HFLSHFLG), on to NOREASON. */
void posupdat_after_branch_negative(void)
{
    tc_to(POSUPDAT + 054u, DOWNFLAG); /* ADRES HFLSHFLG */
}

/* NOREASON: with LRINHBIT set, "UPDATE INHIBITED -- TEST VELOCITY ANYWAY"
 * (VMEASCHK). Else "DO POSITION UPDATE": to the interpreter. */
void noreason(void)
{
    cs(ADDRESS_OF(FLGWRD11));
    mask(LRINHBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        vmeaschk();
        return;
    }
    tc_at(NOREASON + 4u);
    intpret();
}

/* "RESCALE H TO 2(28)M". */
static const word noreason_words[] = {
    /* NOREASON +5 */ ORDERS(DLOAD, SR4),
    ADDRESS_PLUS_ONE(HCALC), /* "RESCALE H TO 2(28)M" */
    ORDER(EXIT),
};

const struct interpretive_code NOREASON_CODE = {033u, NOREASON + 5u, COUNT(noreason_words),
                                                noreason_words};

/* After it: "STORE DELTAH IN MPAC AND" "BRING HCALC INTO A,L", and its
 * signs agreed (ALSIGNAG); ... */
void noreason_after_exit(void)
{
    dca(ADDRESS_OF(DELTAH));
    dxch(ADDRESS_OF(MPAC));
    tc_to(NOREASON + 013u, ALSIGNAG);
}

/* ... "IF HIGH PART OF HCALC IS NON-ZERO, THEN" "HCALC > HMAX," "SO UPDATE
 * IS BYPASSED" (VMEASCHK). Else "FOR LATER SHORTMP" +0 in MPAC +2, and
 * with "HMAX - H" (LRHMAX less the low part) +0 or less, "IF H >HMAX,
 * BYPASS UPDATE"; else "WH(1 - H/HMAX)" (LRWH, LRHMAX) times DELTAH:
 * "DELTAH (WH)(1 - H/HMAX) IN MPAC" (SHORTMP2). */
void noreason_after_alsignag(void)
{
    if (!A_is_zero()) {
        vmeaschk();
        return;
    }
    ts(ADDRESS_OF(MPAC) + 2u);
    cs(L_ADDRESS); /* "-H AT 2(14) M" */
    ad_at(ADDRESS_OF(LRHMAX));
    if (A_is_zero_or_minus()) {
        vmeaschk();
        return;
    }
    mp_at(ADDRESS_OF(LRWH)); /* "WH(HMAX - H)" */
    dv_at(ADDRESS_OF(LRHMAX));
    ts(ADDRESS_OF(MPTEMP));
    tc_to(NOREASON + 031u, SHORTMP2);
}

/* To the interpreter: "MODE IS DP FROM ABOVE". */
void noreason_after_shortmp2(void)
{
    tc_at(NOREASON + 032u);
    intpret();
}

/* "DELTAR = DH(WH)(1 - H/HMAX) UNIT/R/" added to R1S: GNUR, the new
 * position, and its gravity (MUNGRAV). */
static const word noreason_2_words[] = {
    /* NOREASON +33 */ ORDER(SL1),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UNIT_R_), /* "DELTAR = DH(WH)(1 - H/HMAX) UNIT/R/" */
    ADDRESS_PLUS_ONE(R1S),
    STCALL(GNUR),
    ADDRESS(MUNGRAV),
    ORDER(EXIT),
};

const struct interpretive_code NOREASON_2_CODE = {033u, NOREASON + 033u, COUNT(noreason_2_words),
                                                  noreason_2_words};

/* After it: the restart point (QUIKFAZ5), ... */
void noreason_after_second_exit(void)
{
    tc_to(NOREASON + 042u, QUIKFAZ5);
}

/* ... A +0, ... */
void noreason_after_quikfaz5(void)
{
    set_A(ZERO);
    rupdated();
}

/* ... and RUPDATED: the new position into R1S (GNURVST), which returns to
 * VMEASCHK. */
void rupdated(void)
{
    tc_to(RUPDATED, GNURVST);
}

/* ---- VMEASCHK, VELUPDAT -- pp. 886-888 ---- */

/* VMEASCHK: "RESTART AT NEXT LOCATION" (QUIKFAZ5); ... */
void vmeaschk(void)
{
    tc_to(VMEASCHK, QUIKFAZ5);
}

/* ... "IS V READING AVAILABLE?" (VELDABIT): "NO:  SEE IF V READING TO BE
 * TAKEN" (VALTCHK); yes: VELUPDAT. */
void vmeaschk_after_quikfaz5(void)
{
    cs(ADDRESS_OF(FLGWRD11));
    mask(VELDABIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        valtchk();
        return;
    }
    velupdat();
}

/* VELUPDAT: "PROCESS VELOCITY DATA". From VSELECT, the beam: "X1 = -6
 * VSELECT, X2 = -2 VSELECT" (INDEX FIXLOC, the work area's index
 * registers); the landing radar's CDUs (LRYCDU, LRZCDU, LRXCDU, from E
 * bank 4) "IN CDUSPOTS", and "GET SINES AND COSINES FOR NBSM" (QUICTRIG,
 * through BANKCALL). */
void velupdat(void)
{
    cs(ADDRESS_OF(VSELECT));
    ts(L_ADDRESS);
    ads(L_ADDRESS); /* "-2 VSELECT IN L" */
    ad_at(L_ADDRESS);
    ad_at(L_ADDRESS); /* "-6 VSELECT IN A" */
    if (!fixloc_at_a_vac_area()) not_modelled();
    dxch(indexed(ADDRESS_OF(X1), FIXLOC));
    set_A(EBANK4);
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(LRYCDU)); /* "STORE LRCDUS IN CDUSPOTS" */
    ts(ADDRESS_OF(CDUSPOT));
    ca(ADDRESS_OF(LRZCDU));
    ts(ADDRESS_OF(CDUSPOT) + 2u);
    ca(ADDRESS_OF(LRXCDU));
    ts(ADDRESS_OF(CDUSPOT) + 4u);
    tc_to(VELUPDAT + 017u, BANKCALL); /* CADR QUICTRIG */
}

/* After QUICTRIG: "SET PD TO ZERO", and to the interpreter. */
void velupdat_after_quictrig(void)
{
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(PUSHLOC));
    tc_at(VELUPDAT + 023u);
    intpret();
}

/* "CONVERT VBEAM FROM NB TO SM" (the beam's vector, by X1, through
 * *NBSM*), pushed; the measurement VMEAS "SCALE TO M/CS AT 2(6)" (by X2's
 * scale) and pushed. */
static const word velupdat_words[] = {
    /* VELUPDAT +24 */ ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X1(VZBEAMNB, 0), /* "CONVERT VBEAM FROM NB TO SM" */
    ADDRESS(STAR_NBSM_STAR),
    ORDERS(PDDL, SL),        /* "STORE IN PD 0-5" */
    ADDRESS_PLUS_ONE(VMEAS), /* "LOAD VELOCITY MEASUREMENT" */
    SL_COUNT(12),
    ORDERS(DMP_STAR, PUSH),            /* "SCALE TO M/CS AT 2(6)" */
    ADDRESS_PLUS_ONE_AT_X2(VZSCAL, 0), /* "AND STORE IN PD 6-7" */
    ORDER(EXIT),
};

const struct interpretive_code VELUPDAT_CODE = {033u, VELUPDAT + 024u, COUNT(velupdat_words),
                                                velupdat_words};

/* After it: "CHANGE STORE MODE TO VECTOR"; "STORE DELV IN MPAC" (PIPTEM,
 * the PIPAs RDGIMS read, each a vector component's high word); "RESTORE
 * EBANK 7"; to the interpreter. */
void velupdat_after_exit(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(MODE));
    ca(ADDRESS_OF(PIPTEM));
    zl();
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(PIPTEM) + 1u);
    zl();
    dxch(ADDRESS_OF(MPAC) + 3u);
    ca(ADDRESS_OF(PIPTEM) + 2u);
    zl();
    dxch(ADDRESS_OF(MPAC) + 5u);
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_at(VELUPDAT + 052u);
    intpret();
}

/* The velocity at the measurement's time, "VU = V(N-1) + DELVU + G(N-1)
 * DTU", less the moon's rotation (DELVS), along the beam: "DELTAV = VMEAS -
 * V(EST)", and its size against the limit, "ABS(DV) - (7.5 +
 * ABS(VM)/8))". */
static const word velupdat_2_words[] = {
    /* VELUPDAT +53 */ ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(KPIP1),   /* "SCALE DELV TO 2(7) M/CS AND PUSH" */
    ADDRESS_PLUS_ONE(LRVTIME), /* "TIME OF DELV AT 2(28) CS" */
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE(PIPTIME), /* "TU - T(N-1)" */
    ADDRESS_PLUS_ONE(2SEC_28),
    ORDERS(VXSC, VSL1),      /* "G(N-1)(TU - T(N-1))" */
    ADDRESS_PLUS_ONE(GDT_2), /* "SCALED AT 2(7) M/CS" */
    ORDERS(VAD, VAD),        /* "PUSH UP FOR DELV" */
    ADDRESS_PLUS_ONE(V),     /* "VU = V(N-1) + DELVU + G(N-1) DTU" */
    ORDERS(VSL2, VAD),       /* "SCALE TO 2(5) M/CS AND SUBTRACT" */
    ADDRESS_PLUS_ONE(DELVS), /* "MOON ROTATION." */
    ORDERS(PUSH, ABVAL),     /* "STORE IN PD" */
    ORDERS(SR4, DAD),        /* "ABS(VM)/8 + 7.5 AT 2(6)" */
    ADDRESS_PLUS_ONE(7_5),
    STOVL_D(20),           /* "STORE IN 20D AND PICK UP VM" */
    ORDERS(DOT, BDSU),     /* "V(EST) AT 2(6)" */
    ADDRESS_PLUS_ONE_D(0), /* "DELTAV = VMEAS - V(EST)" */
    ORDERS(PUSH, ABS),
    ORDERS(DSU, EXIT), /* "ABS(DV) - (7.5 + ABS(VM)/8))" */
    ADDRESS_PLUS_ONE_D(20),
};

const struct interpretive_code VELUPDAT_2_CODE = {033u, VELUPDAT + 053u, COUNT(velupdat_2_words),
                                                  velupdat_2_words};

/* After it: LRMCTR counted, and BRANCH on the difference: ... */
void velupdat_after_second_exit(void)
{
    incr(ADDRESS_OF(LRMCTR));
    tc_to(VELUPDAT + 0101u, BRANCH);
}

/* ... positive or +0, "DELTA V TOO LARGE.	ALARM" (VFAIL); ... */
void velupdat_after_branch(void)
{
    vfail();
}

void velupdat_after_branch_zero(void)
{
    vfail();
}

/* ... negative: "TURN OFF VEL FAIL LAMP" (VFLSHFLG); ... */
void velupdat_after_branch_negative(void)
{
    tc_to(VELUPDAT + 0104u, DOWNFLAG); /* ADRES VFLSHFLG */
}

/* ... "IF VX INHIBIT RESET, INCORPORATE DATA." (VUPDAT); else "RESET VX
 * INHIBIT" (VXINH), ... */
void velupdat_after_vflshflg(void)
{
    ca(ADDRESS_OF(FLGWRD11));
    mask(VXINHBIT);
    if (A_is_zero()) {
        vupdat();
        return;
    }
    tc_to(VELUPDAT + 0112u, DOWNFLAG); /* ADRES VXINH */
}

/* ... and "IF VSELECT = 2 (X AXIS)." "BYPASS UPDATE" (ENDVDAT); else
 * VUPDAT. */
void velupdat_after_vxinh(void)
{
    ca(ADDRESS_OF(VSELECT));
    ad(NEG2);
    if (A_is_zero()) {
        valtchk(); /* ENDVDAT = VALTCHK */
        return;
    }
    vupdat();
}

/* ---- VUPDAT, USEVF, WSTOR -- pp. 888-889 ---- */

/* VUPDAT: with LRINHBIT set, "UPDATE INHIBITED" (VALTCHK). Else +0 in MPAC
 * +1; "STORE E7 ERASABLES NEEDED IN TEMPS" (ABVEL*, VSELECT*), and to E bank
 * 5 for the weights. "IF V < VF, USE WVF" (USEVF); "IF V > VMAX, W = 0"; else
 * the beam's weight (LRWVZ, by VSELECT*) times "WV( 1 - V/VMAX )". On to
 * WSTOR. */
void vupdat(void)
{
    cs(ADDRESS_OF(FLGWRD11));
    mask(LRINHBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        valtchk(); /* "UPDATE INHIBITED" */
        return;
    }
    ts(ADDRESS_OF(MPAC) + 1u);
    ca(ADDRESS_OF(ABVEL)); /* "STORE E7 ERASABLES NEEDED IN TEMPS" */
    ts(ADDRESS_OF(ABVEL_STAR));
    ca(ADDRESS_OF(VSELECT));
    ts(ADDRESS_OF(VSELECT_STAR));
    set_A(EBANK5);
    ts(EBANK_ADDRESS); /* "CHANGE EBANKS" */
    cs(ADDRESS_OF(LRVF));
    ad_at(ADDRESS_OF(ABVEL_STAR));
    if (A_is_zero_or_minus()) {
        usevf();
        return;
    }
    cs(ADDRESS_OF(ABVEL_STAR));
    ad_at(ADDRESS_OF(LRVMAX)); /* "VMAX - V" */
    if (A_is_zero_or_minus()) {
        set_A(ZERO); /* WSTOR -1 */
        wstor();
        return;
    }
    if (fetch(ADDRESS_OF(VSELECT_STAR)) > 2u) not_modelled();           /* (a beam: 0, 1 or 2) */
    mp_at(indexed(ADDRESS_OF(LRWVZ), fetch(ADDRESS_OF(VSELECT_STAR)))); /* "WV(VMAX - V)" */
    dv_at(ADDRESS_OF(LRVMAX));                                          /* "WV( 1 - V/VMAX )" */
    wstor();
}

/* USEVF: "USE APPROPRIATE CONSTANT WEIGHT" (LRWVFZ, by VSELECT*). */
void usevf(void)
{
    if (fetch(ADDRESS_OF(VSELECT_STAR)) > 2u) not_modelled();
    ca(indexed(ADDRESS_OF(LRWVFZ), fetch(ADDRESS_OF(VSELECT_STAR))));
    wstor();
}

/* WSTOR: the weight to MPAC; "IF IN P65,P66,P67, USE ANOTHER CONSTANT"
 * (LRWVFF, with MODREG over 64); "CHANGE EBANKS" back to 7, and to the
 * interpreter. (A TS that overflows, entered here with A so, skips the
 * CS BIT7.) */
void wstor(void)
{
    if (!ts(ADDRESS_OF(MPAC))) set_A(negate(BIT7)); /* "(=64D)" */
    ad_at(ADDRESS_OF(MODREG));
    if (!A_is_zero_or_minus()) {
        ca(ADDRESS_OF(LRWVFF));
        ts(ADDRESS_OF(MPAC));
    }
    set_A(EBANK7);
    ts(EBANK_ADDRESS); /* "CHANGE EBANKS" */
    tc_at(WSTOR + 011u);
    intpret();
}

/* "W(DELTA V)(VBEAMSM) UP 6-7, 0-5", "ADD WEIGHTED DELTA V TO VELOCITY":
 * GNUV, the new velocity. */
static const word wstor_words[] = {
    /* WSTOR +12 */ ORDERS(DMP, VXSC), /* "W(DELTA V)(VBEAMSM) UP 6-7, 0-5" */
    ORDER(VAD),
    ADDRESS_PLUS_ONE(V1S), /* "ADD WEIGHTED DELTA V TO VELOCITY" */
    STORE(GNUV),
    ORDER(EXIT),
};

const struct interpretive_code WSTOR_CODE = {033u, WSTOR + 012u, COUNT(wstor_words), wstor_words};

/* After it: "DO NOT RE-UPDATE" (QUIKFAZ5); ... */
void wstor_after_exit(void)
{
    tc_to(WSTOR + 017u, QUIKFAZ5);
}

/* ... A six, ... */
void wstor_after_quikfaz5(void)
{
    set_A(SIX);
    vupdated();
}

/* ... and VUPDATED: "STORE NEW VELOCITY VECTOR" (GNURVST), which returns
 * to VALTCHK. */
void vupdated(void)
{
    tc_to(VUPDATED, GNURVST);
}

/* ---- VALTCHK, READV, GNURVST, QUIKFAZ5 -- pp. 889-890 ---- */

/* VALTCHK: "DO NOT REPEAT ABOVE" (QUIKFAZ5); ... */
void valtchk(void)
{
    tc_to(VALTCHK, QUIKFAZ5);
}

/* ... "TEST READVEL TO SEE IF VELOCITY READING" "IS DESIRED." (READVBIT):
 * "YES -- READ VELOCITY" (READV). Else with ABVEL over 2KFT/SEC, "V > 2000
 * FT/SEC  DO NOT READ VEL" (CONTSERV); else "V < 2000 FT/SEC  SET READVEL
 * AND READ." */
void valtchk_after_quikfaz5(void)
{
    set_A(READVBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        readv();
        return;
    }
    cs(ADDRESS_OF(ABVEL));
    ad(K2KFT_SEC);
    if (A_is_zero_or_minus()) {
        contserv();
        return;
    }
    tc_to(VALTCHK + 011u, UPFLAG); /* ADRES READVEL */
}

/* READV: "SET UP JOB TO READ VELOCITY BEAMS." (NOVAC, LRVJOB); ... */
void readv(void)
{
    set_A(PRIO32);
    tc_to(READV + 1u, NOVAC); /* 2CADR LRVJOB */
}

/* ... "CONTINUE WITH SERVICER". */
void readv_after_novac(void)
{
    contserv();
}

/* GNURVST: "STORE GNUR (=GNUV) IN R1S OR V1S": "A = 0 FOR R, A = 6 FOR V"
 * (in BUF, the index); back to the caller. */
void gnurvst(void)
{
    unsigned i;

    if (ts(ADDRESS_OF(BUF))) not_modelled();
    if (fetch(ADDRESS_OF(BUF)) != 0u && fetch(ADDRESS_OF(BUF)) != 6u) not_modelled();
    for (i = 0; i < 6u; i += 2u) {
        dca(ADDRESS_OF(GNUR) + i);
        dxch(indexed(ADDRESS_OF(R1S) + i, fetch(ADDRESS_OF(BUF))));
    }
    return_to_q();
}

/* QUIKFAZ5: the restart point of group 5 moved to the caller's return:
 * "SET EBANK 3", "Q TO A, A TO L", the return in PHSNAME5, the E bank
 * restored, and back (TC A). (A Q with overflow is not modelled: the TS
 * would skip the LXCH, and TC A go to +1 or -1.) */
void quikfaz5(void)
{
    set_A(EBANK3);
    xch(EBANK_ADDRESS); /* "SET EBANK 3" */
    dxch(L_ADDRESS);    /* "Q TO A, A TO L" */
    if (ts(ADDRESS_OF(PHSNAME5))) not_modelled();
    lxch(EBANK_ADDRESS);
    return_to_a();
}

/* ---- HFAIL, NORLITE, VFAIL, NOLITE -- pp. 890-891 ---- */

/* HFAIL: "IF R = 0, DO NOT TURN ON TRK FAIL" (LRRCTR +0: NORLITE). R is
 * the count L (LRLCTR) had at the last failure: with L less R 4 or more
 * (MASK NEG3 not +0), NORLITE; with it under 4, a failure among the last
 * readings before this one, "SET BIT TO TURN ON TRACKER FAIL LITE"
 * (HFLSHFLG), on to NORLITE. */
void hfail(void)
{
    cs(ADDRESS_OF(LRRCTR));
    if (A_is_zero()) {
        norlite();
        return;
    }
    ad_at(ADDRESS_OF(LRLCTR));
    mask(NEG3);
    if (!A_is_zero()) {
        norlite();
        return;
    }
    tc_to(HFAIL + 010u, UPFLAG); /* ADRES HFLSHFLG */
}

/* NORLITE: "SET R = L", and VMEASCHK. */
void norlite(void)
{
    ca(ADDRESS_OF(LRLCTR));
    ts(ADDRESS_OF(LRRCTR));
    vmeaschk();
}

/* VFAIL: "DELTA Q LARGE". "IF S = 0, DO NOT TURN ON TRACKER FAIL"
 * (LRSCTR +0: NOLITE). "TEST FOR M-S > 3" (LRMCTR less LRSCTR, MASK
 * NEG3): if so, NOLITE; if not, "TWO OR MORE OF THE" "LAST FOUR V READINGS
 * WERE BAD,": "SET BIT TO TURN ON TRACKER FAIL LITE" (VFLSHFLG), on to
 * NOLITE. */
void vfail(void)
{
    cs(ADDRESS_OF(LRSCTR));
    if (A_is_zero()) {
        nolite();
        return;
    }
    ad_at(ADDRESS_OF(LRMCTR)); /* "M-S" */
    mask(NEG3);                /* "TEST FOR M-S > 3" */
    if (!A_is_zero()) {
        nolite();
        return;
    }
    tc_to(VFAIL + 010u, UPFLAG); /* ADRES VFLSHFLG */
}

/* NOLITE: "SET S = M"; "TEST FOR Z COMPONENT" (VSELECT): "NOT Z, DO NOT
 * SET VX INHIBIT" (ENDVDAT). Z (+0): "Z COMPONENT - SET FLAG TO SKIP X"
 * (VXINH), as "ERROR MAY BE DUE TO CROSS" "LOBE LOCK UP NOT DETECTED ON X
 * AXIS.", then ENDVDAT. (A negative VSELECT, which the program never
 * holds, skips onto the ADRES: not modelled.) */
void nolite(void)
{
    ca(ADDRESS_OF(LRMCTR));
    ts(ADDRESS_OF(LRSCTR));
    switch (ccs_at(ADDRESS_OF(VSELECT))) {
    case POSITIVE:
    case IS_MINUS_ZERO: valtchk(); return;                 /* ENDVDAT */
    case IS_PLUS_ZERO: tc_to(NOLITE + 4u, UPFLAG); return; /* ADRES VXINH */
    case NEGATIVE: not_modelled();
    }
}

void nolite_after_vxinh(void)
{
    valtchk(); /* ENDVDAT */
}

#endif
