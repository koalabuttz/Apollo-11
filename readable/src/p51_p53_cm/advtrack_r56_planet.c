/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 8 (Comanche055 pp. 778-784): ADVTRACK and
 * ROTA, the line of sight to a point ahead on the ground track (bank 26);
 * S52.3, the preferred orientation (bank 16); R56, the alternate line of
 * sight mark (bank 17); and PLANET, a star's or a planet's direction
 * (bank 15).
 *
 * Each call out of the file (BANKCALL, CLEARMRK, GOTOPOOH) leaves Z at
 * the routine and Q at the word after the TC, where the routine comes
 * back to; GOTOPOOH doesn't come back. The faithful port keys a routine
 * at each such return, at each TC's return in the file, and at each TC
 * INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r56_flash(void);
static void nosam_flash(void);
static void notplan(void);

/* ---- ADVTRACK, ROTA -- pp. 778-779 (bank 26) ---- */

/* ADVTRACK "IS USED TO COMPUTE AN OPTICS LOS VECTOR TO A POINT ON THE
 * GROUND TRACK 60 DEGREES FORWARD OF THE LOCAL VERTICAL OF AN ADVANCED
 * ORBIT A SPECIFIED NUMBER OF REVOLUTIONS FROM NOW". The moon's rotation
 * axis now (RP-TO-R of UNITZ, at the time also in AOPTIME) to STAR; the
 * CSM's state then (CSMCONIC); -UNIT(V x R) kept in 24D, "FOR 2ND
 * ROTATION"; and the line of sight down, -UNIT(R), pushed. */
static const word advtrack_words[] = {
    /* ADVTRACK */ ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, PUSH), /* "INITIALIZE FOR RP-TO-R" */
    ADDRESS_PLUS_ONE(UNITZ),
    ORDERS(RTB, PUSH),
    ADDRESS(LOADTIME),
    STCALL(AOPTIME), /* "TIME ALSO IN AOPTIME FOR CSMCONIC" */
    ADDRESS(RP_TO_R),
    STODL(STAR),
    ADDRESS_PLUS_ONE(AOPTIME),
    STCALL(TDEC1), /* "UPDATE STATE TO TIME" */
    ADDRESS(CSMCONIC),
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(UNIT),
    STOVL_D(24), /* "SAVE -UNIT(VxR) FOR 2ND ROTATION" */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, VCOMP),
    ORDERS(SETPD, PUSH),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(EXIT),
};

const struct interpretive_code ADVTRACK_CODE = {026u, ADVTRACK, COUNT(advtrack_words),
                                                advtrack_words};

/* LANDMARK's low three bits, "GET NUMBER OF ADVANCE PERIODS", times
 * BIT11, "GET N/16", to the work area's 30D. */
void advtrack_periods(void)
{
    ca(ADDRESS_OF(LANDMARK));
    mask(SEVEN);
    mp(BIT11);                /* EXTEND; MP BIT11 */
    xch(L_ADDRESS);           /* XCH L */
    ts(indexed(30u, FIXLOC)); /* INDEX FIXLOC; TS 30D */
    tc_at(ADVTRACK + 035u);
    intpret();
}

/* The line of sight turned about the radius by the moon's turn in those
 * periods (MPERIOD, "APPROX LUNAR ROT ANG IN 2HRS x 16"), then about 24D
 * by 60 degrees less that (DP1/6); "STORE FINAL LOS IN STAR", and back to
 * SR52.1 (COM52). ROTA turns the line of sight in the push-down list about
 * STAR by AOPTIME's angle: LOS COS(A) + (UR x LOS) SIN(A) + (UR . LOS)
 * (1 - COS(A)) UR, unit, pushed again. */
static const word advtrack_rota_words[] = {
    /* ADVTRACK +036 */ ORDERS(SLOAD, DMP),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE(MPERIOD),
    STCALL(AOPTIME), /* "ROTATE ANG ABOUT UR" */
    ADDRESS(ROTA),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_D(24), /* "PICK UP 2ND ROTATION AXIS" */
    STODL(STAR),
    ADDRESS_PLUS_ONE(DP1_6),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(AOPTIME), /* "2ND RAT ANGLE = 60 - A" */
    STCALL(AOPTIME),
    ADDRESS(ROTA), /* "GO ROTATE 2ND TIME" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    STCALL(STAR),   /* "STORE FINAL LOS IN STAR" */
    ADDRESS(COM52), /* "RETURN TO SR52.1" */
    /* ROTA */ ORDERS(DLOAD, SIN),
    ADDRESS_PLUS_ONE(AOPTIME),
    ORDERS(PDVL, VXV), /* "PUSH 1/2SIN(A) PD 6-7" */
    ADDRESS_PLUS_ONE(STAR),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VXSC, VSL2), /* "1/2SIN(A)(URXLOS) PD 6-11" */
    ORDERS(PDVL, DOT),
    ADDRESS_PLUS_ONE(STAR),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VXSC, VSL2),
    ADDRESS_PLUS_ONE(STAR),
    ORDERS(PDDL, COS), /* "1/2(UR . LOS)UR 12-17" */
    ADDRESS_PLUS_ONE(AOPTIME),
    ORDERS(PDVL, BVSU), /* "PUSH 1/2COS(A) 18-19" */
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VXSC, VSL1), /* "UP 18-19" */
    ORDERS(VAD, VAD),
    ORDERS(UNIT, SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PUSH, RVQ),
};

const struct interpretive_code ADVTRACK_ROTA_CODE = {
    026u, ADVTRACK + 036u, COUNT(advtrack_rota_words), advtrack_rota_words};

/* ---- S52.3 -- p. 780 (bank 16) ---- */

/* S52.3, at the "TIME OF ALIGNMENT IN MPAC": the CSM's state then
 * (CSMPREC); "ZSMD= UNIT(-R)", "YSMD= UNIT(V X R)", "XSMD= UNIT(YSMD x
 * ZSMD)"; back (QMAJ). */
static const word s52_3_words[] = {
    /* S52.3 */ ORDER(STQ),
    ADDRESS(QMAJ),
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(UNIT),
    STOVL(ZSMD),
    ADDRESS_PLUS_ONE(VATT),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    STORE(YSMD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(ZSMD),
    STCALL(XSMD),
    ADDRESS(QMAJ),
};

const struct interpretive_code S52_3_CODE = {016u, S52_3, COUNT(s52_3_words), s52_3_words};

/* ---- R56 -- pp. 781-782 (bank 17) ---- */

/* R56, "ALTERNATE LOS SIGHTING MARK ROUTINE", for the backup alignment
 * programs (P53, P54): the caller's return to R53EXIT; the optics angles
 * of the alternate line of sight flashed (V06N94B, GOFLASH): terminate,
 * proceed ("ANGLES OK", R56A) or enter ("NEW ANGLES", the flash again). */
static const word r56_words[] = {
    /* R56 */ ORDERS(STQ, EXIT),
    ADDRESS(R53EXIT),
};

const struct interpretive_code R56_CODE = {017u, R56, COUNT(r56_words), r56_words};

static void r56_flash(void)
{
    set_A(V06N94B);
    tc_to(R56 + 3u, BANKCALL); /* CADR GOFLASH */
}

void r56_after_exit(void)
{
    r56_flash();
}

void r56_terminate(void)
{
    tc_to(R56 + 5u, GOTOPOOH); /* "TERM." */
}

void r56_proceed(void)
{
    tc_at(R56 + 6u); /* TC R56A, "PROCEED: ANGLES OK" */
    r56a();
}

void r56_enter(void)
{
    tc_at(R56 + 7u); /* TC -5, "ENTER: NEW ANGLES" */
    r56_flash();
}

/* R56A: SXTMARK +2, "INHIBIR EXT VB ACT AND FIND VAC AREA"; the display
 * cleared (CLEANDSP); R56A1: V53 flashed, "REQUESTING ALTERNATE MARK"
 * (GOMARK2): V34 terminates, V33 asks again, enter marks. */
void r56a(void)
{
    tc_to(R56A, BANKCALL); /* CADR SXTMARK +2 */
}

void r56a_after_sxtmark(void)
{
    set_A(ZERO);
    tc_to(R56A + 3u, BANKCALL); /* CADR CLEANDSP */
}

void r56a1(void)
{
    set_A(VB53);                 /* "DISPLAY V53 REQUESTING ALTERNATE MARK" */
    tc_to(R56A1 + 1u, BANKCALL); /* CADR GOMARK2 */
}

void r56a1_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH, "V34: TERMINATE" */
}

void r56a1_again(void)
{
    r56a1(); /* TCF R56A1, "V33: DON'T PROCEED" */
}

void r56a1_mark(void)
{
    tc_at(R56A1 + 5u);
    intpret();
}

/* The mark: the optics angles from the mark buffer (MRKBUF1 +3 and +5)
 * to SAC and PAC. */
static const word r56a1_words[] = {
    /* R56A1 +6 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(MRKBUF1, 3),
    STODL(SAC),
    ADDRESS_PLUS_ONE_AT(MRKBUF1, 5),
    STORE(PAC),
    ORDER(EXIT),
};

const struct interpretive_code R56A1_CODE = {017u, R56A1 + 6u, COUNT(r56a1_words), r56a1_words};

/* With interrupts held off, the time (TIME2, TIME1) and the five angles
 * (CDUY, SAC, CDUZ, PAC, CDUX) to the mark area (INDEX MARKSTAT, each
 * time), as SXTMARK stores a mark: "THIS IS A BACKUP SYSTEM MARK". (A TS
 * after a CA never skips.) Then CLEARMRK, "ENABLE EXTENDED VERBS". */
void r56a1_store_mark(void)
{
    static const unsigned angles[] = {ADDRESS_OF(CDUY), ADDRESS_OF(SAC), ADDRESS_OF(CDUZ),
                                      ADDRESS_OF(PAC), ADDRESS_OF(CDUX)};
    unsigned i;

    inhint();
    dca(ADDRESS_OF(TIME2));                              /* EXTEND; DCA TIME2 */
    dxch(indexed_dxch(0u, fetch(ADDRESS_OF(MARKSTAT)))); /* INDEX MARKSTAT; DXCH 0 */
    for (i = 0; i < COUNT(angles); ++i) {
        ca(angles[i]);
        ts(indexed_ts(2u + i, fetch(ADDRESS_OF(MARKSTAT)))); /* INDEX MARKSTAT; TS 2 + i */
    }
    relint();
    tc_to(R56A1 + 041u, CLEARMRK);
}

/* V50N25 with option 16 (GOPERF1, OCT16): terminate, proceed (the mark
 * completed: R56B, the display cleared and the star's code flashed at
 * R53C1) or enter (recycle, another mark: R56A +2). */
void r56a1_after_clearmrk(void)
{
    set_A(OCT16);
    tc_to(R56A1 + 043u, BANKCALL); /* CADR GOPERF1 */
}

void r56a1_marked_terminate(void)
{
    tc_to(R56A1 + 045u, GOTOPOOH); /* "TERM." */
}

void r56a1_marked_proceed(void)
{
    tc_to(R56B, BANKCALL); /* TCF R56B; TC BANKCALL; CADR R53C1 */
}

void r56a1_marked_enter(void)
{
    r56a_after_sxtmark(); /* TCF R56A +2 */
}

/* ---- PLANET, NOSAM, CORPLAN -- pp. 782-784 (bank 15) ---- */

/* PLANET, at the sighting time (MPAC, to TSIGHT): the caller's return to
 * QMIN; LOCSAM, the sun, the earth and the moon then (VSUN, VEARTH,
 * VMOON), with VEARTH and VSUN swapped. */
static const word planet_words[] = {
    /* PLANET */ STORE(TSIGHT),
    ORDERS(STQ, CALL),
    ADDRESS(QMIN),
    ADDRESS(LOCSAM),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VEARTH),
    STOVL_D(0),
    ADDRESS_PLUS_ONE(VSUN),
    STOVL(VEARTH),
    ADDRESS_PLUS_ONE_D(0),
    STORE(VSUN),
    /* NOSAM */ ORDER(EXIT),
};

const struct interpretive_code PLANET_CODE = {015u, PLANET, COUNT(planet_words), planet_words};

/* NOSAM: the code (its low six bits: CS HIGH9; MASK STARCODE) times
 * SIGHTSIX to BESTI or BESTJ (INDEX STARIND). A star or the sun, earth or
 * moon (positive): NOTPLAN. A planet (code 0): its direction flashed
 * (VNPLANV, GOFLASH), for the crew to load: terminate, proceed or enter
 * (the flash again). (A is L, a word: the TS does not skip; nor is it
 * negative: the CCS takes its first two ways only.) */
void nosam_after_exit(void)
{
    set_A(negate(HIGH9)); /* CS HIGH9 */
    mask_at(ADDRESS_OF(STARCODE));
    mp(SIGHTSIX);                                                  /* EXTEND; MP SIGHTSIX */
    xch(L_ADDRESS);                                                /* XCH L */
    ts(indexed_ts(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    switch (ccs_register(A_ADDRESS)) {                             /* CCS A */
    case POSITIVE: notplan(); return;                              /* TCF NOTPLAN */
    case IS_PLUS_ZERO: nosam_flash(); return;
    default: not_modelled();
    }
}

static void nosam_flash(void)
{
    set_A(VNPLANV);
    tc_to(NOSAM + 013u, BANKCALL); /* CADR GOFLASH */
}

void nosam_terminate(void)
{
    tc_to(NOSAM + 015u, GOTOPOOH);
}

void nosam_proceed(void)
{
    tc_at(NOSAM + 016u); /* TC +2 */
    nosam_planet();
}

void nosam_enter(void)
{
    tc_at(NOSAM + 017u); /* TC -5 */
    nosam_flash();
}

void nosam_planet(void)
{
    tc_at(NOSAM + 020u);
    intpret();
}

/* The planet's direction keyed in (STARSAV3), times 1/SQR3, unit. */
static const word nosam_words[] = {
    /* NOSAM +021 */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(STARSAV3),
    ADDRESS_PLUS_ONE(1_SQR3),
    ORDERS(UNIT, GOTO),
    ADDRESS(CORPLAN),
};

const struct interpretive_code NOSAM_CODE = {015u, NOSAM + 021u, COUNT(nosam_words), nosam_words};

/* NOTPLAN: the code times 6, less one (CCS), against DEC227: past it (the
 * sun, the earth or the moon, codes 38 to 40), CALSAM1; else a star: its
 * index (BESTI or BESTJ) to X1 (INDEX FIXLOC), and its vector from the
 * catalog (CATLOG,1). (A is a word: TS does not skip.) */
static void notplan(void)
{
    cs(A_ADDRESS);
    ad(DEC227);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF CALSAM1 */
        tc_at(CALSAM1);
        intpret();
        return;
    }
    ca(indexed_ca(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    ts(indexed_ts(ADDRESS_OF(X1), FIXLOC));                        /* INDEX FIXLOC */
    tc_at(NOTPLAN + 010u);
    intpret();
}

static const word notplan_words[] = {
    /* NOTPLAN +011 */ ORDERS(VLOAD_STAR, GOTO),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS(CORPLAN),
};

const struct interpretive_code NOTPLAN_CODE = {015u, NOTPLAN + 011u, COUNT(notplan_words),
                                               notplan_words};

/* CALSAM1: the sun's, the earth's or the moon's direction, from LOCSAM's
 * (STARAD -228D on, by the code times 6). CORPLAN: the aberration
 * correction (VEL/C added), unit; back (QMIN). */
static const word calsam1_words[] = {
    /* CALSAM1 +1 */ ORDERS(LXC_1, DLOAD_STAR),
    ADDRESS(STARIND),
    ADDRESS_PLUS_ONE_AT_X1(BESTI, 0),
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(MPAC),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, -228),
    /* CORPLAN */ ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(VEL_C),
    ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code CALSAM1_CODE = {015u, CALSAM1 + 1u, COUNT(calsam1_words),
                                               calsam1_words};

#endif
