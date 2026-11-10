/* SPDX-License-Identifier: MIT */
/* IMU_CALIBRATION_AND_ALIGNMENT.agc of the CM, the basic code of pages
 * 449-454: the "OPTICAL VERIFICATION ROUTINES FOR GYROCOMPASS", and the
 * displays of the latitude and azimuth (LATAZCHK) and of the launch
 * azimuth (AZMTHCG1).
 *
 * GCOMPVER (V65) checks the gyrocompassed platform against two ground
 * targets: OPTDATA asks the crew for each target's azimuth and elevation
 * (TAZEL1), TARGDRVE drives the sextant to the target and takes a mark
 * (SXTMARK, RETARG), and GCOMP4 shows the platform's error found from the
 * two marks, which a proceed adds to the earth rate (ERCOMP1). GCOMP5 ends
 * the verification: back to P02. The faithful C is
 * c64-port/src/cm_imu_calibration.c.
 *
 * Every word is in bank 33 but LATAZCHK's, in bank 4. The erasable words
 * are EBANK 5's but those of the unswitched banks (DSPTEM1, MARKSTAT,
 * QPRET, STARCODE ...). The interpretive code among the basic (TAR/EREF,
 * NEXTBNKS, LITTLSUB ...) is written in the listing's notation: tables the
 * readable interpreter runs (src/interpreter/notation.h). */
#include "imu_calibration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A data word the run has reached (a VN constant): the faithful port does
 * not run it. */
void data_word(void)
{
    not_modelled();
}

/* ---- GCOMPVER and OPTDATA -- p. 449 ---- */

/* GCOMPVER -- p. 449, "ENTERED BY VERB 65 ENTER": the phase (PHASCHNG,
 * OCT 00154). */
void gcompver(void)
{
    tc_at(GCOMPVER);
    continue_at(PHASCHNG);
}

/* GCOMPVER +2: major mode 03 (NEWMODEX). */
void gcompver_after_phaschng(void)
{
    tc_at(GCOMPVER + 2u);
    continue_at(NEWMODEX); /* MM 03 */
}

/* SETNBPOS -- p. 449. The navigation base position (NBPOSPL), which comes
 * back here when verifying. */
void setnbpos(void)
{
    tc_at(SETNBPOS);
    nbpospl();
}

/* SETNBPOS +1: release the display (MKRELEAS), and OPTDATA. */
void setnbpos_after_nbpospl(void)
{
    tc_at(SETNBPOS + 1u);
    continue_at(BANKCALL); /* CADR MKRELEAS */
}

/* OPTDATA +010: show the target's azimuth and elevation (V05 N30,
 * GODSPRET). */
void optdata_display(void)
{
    set_A(V05N30E);
    tc_at(OPTDATA + 011u);
    continue_at(BANKCALL); /* CADR GODSPRET */
}

/* The pass's two words of TAZEL1 (INDEX RUN; the DCA and DXCH address
 * the word after the pair's first): its azimuth and elevation. */
static unsigned tazel1_pair(enum indexed_use use)
{
    unsigned at = indexed_erasable(E(TAZEL1) + 1u, fetch(E(RUN)), use);

    if (at == 0u) not_modelled();
    return at - 1u;
}

/* OPTDATA +2: RUN the pass (L: "1ST PASS=0 2ND PASS=2"), DSPTEM1 +2 one,
 * and DSPTEM1 the pass's azimuth and elevation (INDEX RUN; DCA TAZEL1). */
static void optdata_plus2(void)
{
    lxch(E(RUN));
    ts(ADDRESS_OF(DSPTEM1) + 2u);
    dca(tazel1_pair(INDEXED_READ));
    dxch(ADDRESS_OF(DSPTEM1));
    optdata_display();
}

/* OPTDATA -- p. 449. "CALLS FOR AZIMUTH AND ELEVATION OF TARGE T 1,THEN
 * TARGET 2": A one, L +0 (ZL). */
void optdata(void)
{
    set_A(ONE);
    zl();
    optdata_plus2();
}

/* OPTDATA +013: flash them for the crew to load (V06 N41, GOFLASH). */
void optdata_after_godspret(void)
{
    set_A(VN0641);
    tc_at(OPTDATA + 014u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* OPTDATA +016: terminate, GCOMP5. */
void optdata_terminate(void)
{
    tc_at(OPTDATA + 016u);
    gcomp5();
}

/* OPTDATA +022 -- p. 449. Keep what the crew loaded ("TAZEL1 TARGET 1
 * AZIMUTH", "TAZEL1 +2 TARGET 2 AZIMUTH"); after the first pass, the
 * second (L two); after the second, CONTIN33. */
static void optdata_store(void)
{
    dxch(ADDRESS_OF(DSPTEM1));
    dxch(tazel1_pair(INDEXED_EXCHANGE));
    switch (ccs_at(E(RUN))) {
    case POSITIVE: /* TCF +4 */
        tc_at(OPTDATA + 032u);
        contin33();
        return;
    case IS_PLUS_ZERO:
        set_A(TWO);
        ts(L_ADDRESS);
        break;
    case NEGATIVE: ts(L_ADDRESS); break;
    case IS_MINUS_ZERO: break;
    }
    optdata_plus2(); /* TCF OPTDATA +2 */
}

/* OPTDATA +017: proceed. */
void optdata_proceed(void)
{
    tc_at(OPTDATA + 017u);
    optdata_store();
}

/* OPTDATA +020: enter, show them again (TC -8D). */
void optdata_enter(void)
{
    tc_at(OPTDATA + 020u);
    optdata_display();
}

/* TAR/EREF -1 -- p. 449. "UNDYNAMIC ASSEMBLER": the interpreter at
 * TAR/EREF, the targets' vectors from TAZEL1. */
void tar_eref_minus1(void)
{
    tc_at(TAR_EREF - 1u);
    intpret();
}

/* TAR/EREF -- p. 449, the "TARGET VECTOR" of each target: "SIN(EL)
 * -COS(AZ)COS(EL) SIN(AZ)COS(EL" from its elevation and azimuth in TAZEL1,
 * stored by X2 in the push-down list (18D-22D for the first target, 12D-16D
 * for the second). The pass of TAR1 runs twice: "X1=2 X2=12 S2=6 X1=0
 * X2=6 S2=6". */
/* clang-format off */
static const word tar_eref_words[] = {
    ORDERS(AXT_1, AXT_2), /* "TARGET VECTOR" */
    NUMBER(2),
    NUMBER(12),
    ORDERS(SSP, SETPD),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(6),
    ADDRESS_PLUS_ONE_D(0),
    /* TAR1 */ ORDERS(SLOAD_STAR, SR2), /* "X1=2 X2=12 S2=6 X1=0 X2=6 S2=6" */
    ADDRESS_PLUS_ONE_AT_X1(TAZEL1, 3),
    STORE_D(0), /* "PD00 ELEVATION PD00" */
    ORDER(SIN),
    STORE_X2_D(18), /* "PD06 *** SIN(EL) ***PD12" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(COS, PUSH), /* "PD00 COS(EL) PD00" */
    ORDERS(SLOAD_STAR, RTB),
    ADDRESS_PLUS_ONE_AT_X1(TAZEL1, 2),
    ADDRESS(CDULOGIC),
    STORE_D(2), /* "PD02 AZIMUTH PD02" */
    ORDERS(SIN, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(SL1),
    STORE_X2_D(22), /* "PD10 *** SIN(AZ)COS(EL) ***PD16" */
    ORDERS(DLOAD, COS),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DMP, SL1),
    ORDERS(DCOMP, AXT_1),
    NUMBER(0),
    STORE_X2_D(20), /* "PD08 *** -COS(AZ)COS(EL) ***PD14" */
    ORDERS(TIX_2, RVQ),
    ADDRESS(TAR1),
};
/* clang-format on */

const struct interpretive_code TAR_EREF_CODE = {033u, TAR_EREF, COUNT(tar_eref_words),
                                                tar_eref_words};

/* ---- The targets: CONTIN33 to GTSOPTCS -- pp. 450-451 ---- */

/* CONTIN33 -- p. 450. The first target (STARCODE 1, TARGDRVE's index +0). */
void contin33(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(STARCODE));
    set_A(ZERO);
    tc_at(CONTIN33 + 3u);
    targdrve();
}

/* CONTIN33 +4: the interpreter (NEXTBNKS): the line of sight to it in
 * stable member axes (LITTLSUB) into LOSVEC. */
void contin33_after_targdrve(void)
{
    tc_at(CONTIN33 + 4u);
    intpret();
}

/* CONTIN33 +5 -- p. 450. The targets' vectors (TAR/EREF) again; NEXTBNKS:
 * the first target's in stable member axes (XSM) into STARAD, the second's
 * into STARAD +6, and the line of sight of the mark (LITTLSUB) into
 * LOSVEC. */
/* clang-format off */
static const word nextbnks_words[] = {
    ORDER(CALL), /* the targets' vectors */
    ADDRESS(TAR_EREF),
    /* NEXTBNKS */ ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE(XSM),
    ORDER(VSL1),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(XSM),
    STCALL_AT(STARAD, 6),
    ADDRESS(LITTLSUB),
    STORE(LOSVEC),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code NEXTBNKS_CODE = {033u, CONTIN33 + 5u, COUNT(nextbnks_words),
                                                nextbnks_words};

/* NEXTBNKS +014 -- p. 451, after the EXIT: release the display
 * (MKRELEAS). */
void nextbnks_after_exit(void)
{
    tc_at(NEXTBNKS + 014u);
    continue_at(BANKCALL); /* CADR MKRELEAS */
}

/* NEXBNKSS -- p. 451. The second target (STARCODE 2, index six). */
void nexbnkss(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(STARCODE));
    set_A(SIX);
    tc_at(NEXBNKSS + 3u);
    targdrve();
}

/* NEXBNKSS +4: the interpreter: the platform's error from the two lines
 * of sight (AXISGEN, CALCGTA). */
void nexbnkss_after_targdrve(void)
{
    tc_at(NEXBNKSS + 4u);
    intpret();
}

/* NEXBNKSS +5 -- p. 451. The second mark's line of sight (LITTLSUB) to
 * 12D, the first's (LOSVEC) to 06D; AXISGEN, and CALCGTA the error. */
/* clang-format off */
static const word nexbnkss_words[] = {
    ORDER(CALL), /* the second mark's line of sight */
    ADDRESS(LITTLSUB),
    STOVL_D(12),
    ADDRESS_PLUS_ONE(LOSVEC),
    STCALL_D(6),
    ADDRESS(AXISGEN),
    ORDER(CALL),
    ADDRESS(CALCGTA),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code NEXBNKSS_CODE = {033u, NEXBNKSS + 5u, COUNT(nexbnkss_words),
                                                nexbnkss_words};

/* GCOMP4 -- p. 451. Flash the error (V06 N93, GOFLASH). */
void gcomp4(void)
{
    set_A(V06N93S);
    tc_at(GCOMP4 + 1u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* GCOMP4 +3: terminate, GCOMP5. */
void gcomp4_terminate(void)
{
    tc_at(GCOMP4 + 3u);
    gcomp5();
}

/* GCOMP4 +4: proceed (TCF +2), the interpreter at +6 adds the error OGC
 * into the earth rate ERCOMP1. */
void gcomp4_proceed(void)
{
    tc_at(GCOMP4 + 6u);
    intpret();
}

/* GCOMP4 +7 -- p. 451. ERCOMP1 gains OGC. */
/* clang-format off */
static const word gcomp4_words[] = {
    ORDERS(VLOAD, VAD), /* the error, into the earth rate */
    ADDRESS_PLUS_ONE(OGC),
    ADDRESS_PLUS_ONE(ERCOMP1),
    STORE(ERCOMP1),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code GCOMP4_CODE = {033u, GCOMP4 + 7u, COUNT(gcomp4_words), gcomp4_words};

/* GCOMP4 +5: enter, show it again (TCF GCOMP4). */
void gcomp4_enter(void)
{
    gcomp4();
}

/* GCOMP5 -- p. 451. The end: release the display (MKRELEAS). */
void gcomp5(void)
{
    tc_at(GCOMP5);
    continue_at(BANKCALL); /* CADR MKRELEAS */
}

/* GCOMP5 +2: TRM03FLG reset (DOWNFLAG). */
void gcomp5_after_mkreleas(void)
{
    tc_at(GCOMP5 + 2u);
    continue_at(DOWNFLAG); /* ADRES TRM03FLG */
}

/* GCOMP5 +4: back to major mode 02 (NEWMODEX). */
void gcomp5_after_downflag(void)
{
    tc_at(GCOMP5 + 4u);
    continue_at(NEWMODEX); /* MM 02 */
}

/* GCOMP5 +6: the phase (PHASCHNG, OCT 00004). */
void gcomp5_after_newmodex(void)
{
    tc_at(GCOMP5 + 6u);
    continue_at(PHASCHNG);
}

/* GCOMP5 +010: ENDOFJOB. */
void gcomp5_after_phaschng(void)
{
    tc_at(GCOMP5 + 010u);
    continue_at(ENDOFJOB);
}

/* GTSOPTCS -- p. 451. The optics would not stall: alarm 01602
 * (GTSOPTSS). */
static void gtsoptcs(void)
{
    tc_at(GTSOPTCS);
    continue_at(ALARM);
}

/* GTSOPTCS +2: then GCOMP5. */
void gtsoptcs_after_alarm(void)
{
    tc_at(GTSOPTCS + 2u);
    gcomp5();
}

/* ---- LATAZCHK, in bank 4 -- p. 452 ---- */

/* LATAZCHK -- p. 452, interpretive from its first word: "CALLS FOR
 * AZIMUTH AND LATITUDE". The latitude (shifted left two) to DSPTEM1 +1,
 * and the azimuth in MPAC as a single-precision angle (1STO2S). */
/* clang-format off */
static const word latazchk_words[] = {
    ORDERS(DLOAD, SL2), /* "CALLS FOR AZIMUTH AND LATITUDE" */
    ADDRESS_PLUS_ONE(LATITUDE),
    STODL_AT(DSPTEM1, 1),
    ADDRESS_PLUS_ONE(AZIMUTH),
    ORDERS(RTB, EXIT),
    ADDRESS(1STO2S),
};
/* clang-format on */

const struct interpretive_code LATAZCHK_CODE = {04u, LATAZCHK, COUNT(latazchk_words),
                                                latazchk_words};

/* LATAZCHK +6 -- p. 452, after the EXIT: "CALLS FOR AZIMUTH AND
 * LATITUDE". DSPTEM1 the azimuth in degrees (MPAC), DSPTEM1 +1 the
 * latitude; clear the display (CLEANDSP). */
void latazchk_after_exit(void)
{
    xch(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(DSPTEM1));
    tc_at(LATAZCHK + 010u);
    continue_at(BANKCALL); /* CADR CLEANDSP */
}

/* LATAZCHK +012: flash them (V06 N41, GOFLASH). */
void latazchk_flash(void)
{
    set_A(VNG0641);
    tc_at(LATAZCHK + 013u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* LATAZCHK +015: terminate, "NOT ALLOWED" (TC +2; TC -5): flash again. */
void latazchk_terminate(void)
{
    tc_at(LATAZCHK + 015u);
    tc_at(LATAZCHK + 017u);
    latazchk_flash();
}

/* LATAZCHK +017: enter, flash again (TC -5). */
void latazchk_enter(void)
{
    tc_at(LATAZCHK + 017u);
    latazchk_flash();
}

/* LATAZCHK +020: the interpreter keeps what the crew loaded (AZIMUTH,
 * LATITUDE). */
void latazchk_load(void)
{
    tc_at(LATAZCHK + 020u);
    intpret();
}

/* LATAZCHK +021 -- p. 452. AZIMUTH the azimuth loaded (CDULOGIC), LATITUDE
 * the latitude (shifted right two); back to the caller. */
/* clang-format off */
static const word latazchk_load_words[] = {
    ORDERS(SLOAD, RTB), /* the azimuth loaded */
    ADDRESS_PLUS_ONE(DSPTEM1),
    ADDRESS(CDULOGIC),
    STORE(AZIMUTH),
    ORDERS(SLOAD, SR2),
    ADDRESS_PLUS_ONE_AT(DSPTEM1, 1),
    STORE(LATITUDE),
    ORDER(RVQ),
};
/* clang-format on */

const struct interpretive_code LATAZCHK_LOAD_CODE = {
    04u, LATAZCHK + 021u, COUNT(latazchk_load_words), latazchk_load_words};

/* LATAZCHK +016: proceed (TC +2). */
void latazchk_proceed(void)
{
    tc_at(LATAZCHK + 016u);
    latazchk_load();
}

/* ---- TARGDRVE and RETARG -- pp. 452-453 ---- */

/* RETARG -- p. 453. OPTIND +0, and a mark (SXTMARK, A one). */
static void retarg(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(OPTIND));
    set_A(ONE);
    tc_at(RETARG + 3u);
    continue_at(BANKCALL); /* CADR SXTMARK */
}

/* TARGDRVE -- p. 452. The return into QPLAC, TARG1/2 the target's index,
 * and the interpreter: the target's vector (TAR/EREF) and the sextant
 * angles to it (SXTANG). (A TS that skips would run the CALL after it, a
 * data word.) */
void targdrve(void)
{
    qxch(E(QPLAC));
    if (ts(E(TARG1_2))) not_modelled();
    tc_at(TARGDRVE + 3u);
    intpret();
}

/* TARGDRVE +4 -- p. 452. The targets' vectors (TAR/EREF); X1 minus the
 * target's index (TARG1/2), its vector (6D,1) into STAR, and the sextant
 * angles to it (SXTANG). */
/* clang-format off */
static const word targdrve_words[] = {
    ORDER(CALL), /* the targets' vectors */
    ADDRESS(TAR_EREF),
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(TARG1_2),
    ADDRESS_PLUS_ONE_X1(6),
    STCALL(STAR),
    ADDRESS(SXTANG),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code TARGDRVE_CODE = {033u, TARGDRVE + 4u, COUNT(targdrve_words),
                                                targdrve_words};

/* TARGDRVE +014, after the EXIT: drive the sextant to them (DESOPTS,
 * DESOPTT), and RETARG. */
void targdrve_after_exit(void)
{
    ca(E(SAC));
    ts(ADDRESS_OF(DESOPTS));
    ca(E(PAC));
    ts(ADDRESS_OF(DESOPTT));
    retarg();
}

/* RETARG +5: wait for the optics (OPTSTALL). */
void retarg_after_sxtmark(void)
{
    tc_at(RETARG + 5u);
    continue_at(BANKCALL); /* CADR OPTSTALL */
}

/* RETARG +7: OPTSTALL's error return, GTSOPTCS. */
void retarg_bad_stall(void)
{
    tc_at(RETARG + 7u);
    gtsoptcs();
}

/* RETARG1 -- p. 453. "RELEASE PREVIOUSLY GRABBED VAC AREA": MARKSTAT +0,
 * and the VAC area's first word +0 (INDEX A; TS A, the area's address
 * less one after the CCS); "GO DO SXTMARK AGAIN". */
static void retarg1(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(MARKSTAT));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: ts(indexed_erasable(0u, a_word(), INDEXED_STORE)); break;
    case IS_PLUS_ZERO: ts(A_ADDRESS); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); return; /* (PIPASC, data) */
    }
    retarg(); /* TCF RETARG */
}

/* RETARG +016: a mark taken (its QPRET word nonzero) goes back through
 * QPLAC; none, RETARG1. */
static void retarg_mark_taken(void)
{
    if (A_is_zero()) { /* EXTEND; BZF RETARG1 */
        retarg1();
        return;
    }
    tc_return(E(QPLAC));
}

/* RETARG +014: the mark's QPRET word (INDEX MARKSTAT). */
void retarg_mark(void)
{
    ca(indexed_erasable(ADDRESS_OF(QPRET), fetch(ADDRESS_OF(MARKSTAT)), INDEXED_READ));
    retarg_mark_taken();
}

/* RETARG +010: a terminate (TRM03BIT of FLAGWRD1) ends it, GCOMP5. */
void retarg_after_optstall(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRM03BIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        tc_at(RETARG + 013u);
        gcomp5();
        return;
    case IS_PLUS_ZERO: retarg_mark(); return;
    case NEGATIVE: ca(ADDRESS_OF(QPRET)); break;
    case IS_MINUS_ZERO: break;
    }
    retarg_mark_taken();
}

/* ---- LITTLSUB and AZMTHCG1 -- p. 454 ---- */

/* LITTLSUB -- p. 454. The line of sight of the mark: the return in QPLAC;
 * X1 minus the mark's VAC area (MARKSTAT), its CDU angles (2,1) to CDUSPOT;
 * the sextant's line of sight in navigation base axes (SXTNB), then in
 * stable member axes (TRG*NBSM); back through QPLAC. The EXIT after it
 * nothing reaches but a QPLAC that points there. */
/* clang-format off */
static const word littlsub_words[] = {
    ORDER(STQ), /* the return */
    ADDRESS(QPLAC),
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(MARKSTAT),
    ADDRESS_PLUS_ONE_X1(2),
    STCALL(CDUSPOT),
    ADDRESS(SXTNB),
    ORDER(CALL),
    ADDRESS(TRG_STAR_NBSM),
    ORDER(GOTO),
    ADDRESS(QPLAC),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code LITTLSUB_CODE = {033u, LITTLSUB, COUNT(littlsub_words),
                                                littlsub_words};

/* AZMTHCG1 -- p. 454. The interpreter: the new azimuth NEWAZMTH in
 * degrees. */
void azmthcg1(void)
{
    tc_at(AZMTHCG1);
    intpret();
}

/* AZMTHCG1 +1 -- p. 454. */
/* clang-format off */
static const word azmthcg1_words[] = {
    ORDERS(DLOAD, RTB), /* the azimuth, as a single-precision angle */
    ADDRESS_PLUS_ONE(NEWAZMTH),
    ADDRESS(1STO2S),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code AZMTHCG1_CODE = {033u, AZMTHCG1 + 1u, COUNT(azmthcg1_words),
                                                azmthcg1_words};

/* AZMTHCG1 +5, after the EXIT: DSPTEM1 it; clear the display
 * (CLEANDSP). */
void azmthcg1_after_exit(void)
{
    xch(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(DSPTEM1));
    tc_at(AZMTHCG1 + 7u);
    continue_at(BANKCALL); /* CADR CLEANDSP */
}

/* AZMTHCG1 +011: flash it (V06 N29, GOFLASH). */
void azmthcg1_flash(void)
{
    set_A(VN0629);
    tc_at(AZMTHCG1 + 012u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* AZMTHCG1 +017: the interpreter keeps the azimuth the crew loaded
 * (LAUNCHAZ). */
static void azmthcg1_load(void)
{
    tc_at(AZMTHCG1 + 017u);
    intpret();
}

/* AZMTHCG1 +020 -- p. 454. */
/* clang-format off */
static const word azmthcg1_load_words[] = {
    ORDERS(SLOAD, RTB), /* the azimuth loaded */
    ADDRESS_PLUS_ONE(DSPTEM1),
    ADDRESS(CDULOGIC),
    STORE(LAUNCHAZ),
    ORDER(EXIT),
};
/* clang-format on */

const struct interpretive_code AZMTHCG1_LOAD_CODE = {
    033u, AZMTHCG1 + 020u, COUNT(azmthcg1_load_words), azmthcg1_load_words};

/* AZMTHCG1 +014: terminate (TCF +2; TCF -5): flash again. */
void azmthcg1_terminate(void)
{
    azmthcg1_flash();
}

/* AZMTHCG1 +015: proceed (TCF +2). */
void azmthcg1_proceed(void)
{
    azmthcg1_load();
}

/* AZMTHCG1 +016: enter (TCF -5): flash again. */
void azmthcg1_enter(void)
{
    azmthcg1_flash();
}

/* AZMTHCG1 +025, after the EXIT: PREMTRXC +0 (the azimuth changed), and
 * the phase (PHASCHNG, OCT 00004). */
void azmthcg1_after_load(void)
{
    set_A(ZERO);
    ts(E(PREMTRXC));
    tc_at(AZMTHCG1 + 027u);
    continue_at(PHASCHNG);
}

/* AZMTHCG1 +031: back to Pinball (POSTJUMP, PINBRNCH). */
void azmthcg1_after_phaschng(void)
{
    tc_at(AZMTHCG1 + 031u);
    continue_at(POSTJUMP); /* CADR PINBRNCH */
}

#endif
