/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 1 (Luminary099 pp. 658-663, Comanche055 pp.
 * 461-465): P34 and P74, "TRANSFER PHASE INITIATION", the same program
 * in both missions (bank 35). Its basic code, and its interpretive code in
 * the listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/p34_routines.c). The
 * faithful C is c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * P34 is the active vehicle's program, P74 the passive vehicle's: each
 * sets the active vehicle flag (AVFLAGA, AVFLAGP) and goes on at
 * P34/P74A. The astronaut gives TTPI, the time of TPI, and the elevation
 * angle of the line of sight there (zero to have the program find the
 * angle from the time, ETPIFLAG clear; or else the time from the angle)
 * and the central angle of transfer. The program then works out, for TPI
 * and for the intercept at the end of the transfer, the delta V of each
 * burn and the orbit after TPI, shows them, and goes round again until
 * the astronaut picks the burn program.
 *
 * Each call out of the file (AVFLAGA, AVFLAGP, P20FLGON, ALARM,
 * BANKCALL, GOTOPOOH) leaves Z at the routine and Q at the word after the
 * TC. VNPOOH and DISPLAYE, the file's own (displays.c), run on to their
 * flash, and come back there. The CM's
 * code differs in one place: it zeroes NN with an interpretive SSP where
 * the LM's basic code also sets the central angle to 130 degrees. */
#include "p34_p35.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* The TC DISPLAYE after the first display (the CM's code is three words
 * shorter there, and one word longer in interpretive code); the
 * TC INTPRET after it, and the interpretive code from there. */
#if LUMINARY099
#define DISPLAYE_TC (P34_P74A + 010u)
#else
#define DISPLAYE_TC (P34_P74A + 7u)
#endif
#define TTPI_FIRST (DISPLAYE_TC + 2u)

/* ---- P34, P74 and P34/P74A -- LM p. 661, CM p. 463 ---- */

/* P34: this vehicle the active one (AVFLAGA). */
void p34(void)
{
    tc_to(P34, AVFLAGA);
}

/* P34 +1: on at P34/P74A. */
void p34_after_avflaga(void)
{
    tc_at(P34 + 1u);
    p34_p74a();
}

/* P74: the other vehicle the active one (AVFLAGP), which comes back to
 * P34/P74A. */
void p74(void)
{
    tc_to(P74, AVFLAGP);
}

/* P34/P74A: P20's flags, "SET UPDATFLG, TRACKFLG" (P20FLGON). */
void p34_p74a(void)
{
    tc_to(P34_P74A, P20FLGON);
}

/* P34/P74A +1: "TTPI", verb 06 noun 37, through VNPOOH. */
void p34_p74a_after_p20flgon(void)
{
    set_A(V06N37);
    tc_at(P34_P74A + 2u);
    vnpooh();
}

#if LUMINARY099
/* P34/P74A +3: the central angle 130 degrees (DCA 130DEG, DXCH CENTANG)
 * and NN zero, then "ELEV AND CENTANG" shown (DISPLAYE). */
void p34_p74a_after_vnpooh(void)
{
    dca(DEG130); /* EXTEND; DCA */
    dxch(ADDRESS_OF(CENTANG));
    ca(P30ZERO);
    ts(ADDRESS_OF(NN));
    tc_at(DISPLAYE_TC);
    displaye();
}
#else
/* P34/P74A +3: to the interpreter, for P34_NN_CODE. */
void p34_p74a_after_vnpooh(void)
{
    tc_at(P34_P74A + 3u);
    intpret();
}

/* P34_NN_CODE: NN zero (SSP), and back to basic code. */
static const word p34_nn_words[] = {
    ORDERS(SSP, EXIT),
    ADDRESS_PLUS_ONE(NN),
    NUMBER(0),
};

/* P34/P74A +7, after the EXIT: "ELEV AND CENTANG" shown (DISPLAYE). */
void p34_p74a_after_nn(void)
{
    tc_at(DISPLAYE_TC);
    displaye();
}
#endif

/* After DISPLAYE: to the interpreter, for P34_TTPI_CODE. */
void p34_p74a_after_displaye(void)
{
    tc_at(DISPLAYE_TC + 1u);
    intpret();
}

/* ---- The search for TPI -- LM pp. 661-662, CM pp. 463-464 ---- */

/* P34_TTPI_CODE: ETPIFLAG clear, and TTPI to TIG; with an elevation
 * angle given (ELEV not zero), ETPIFLAG set: the program is to find the
 * time of TPI from the angle. P34/P74B: SELECTMU, the Earth's or the
 * Moon's. P34/P74C: NOMTPI zero, and ITSWICH set; with ETPIFLAG set,
 * cleared again (SWCHSET). INTLOOP: both vehicles to TTPI plus NOMTPI
 * (PRECSET), and S33/34.1, which finds the time of TPI from the angle, or
 * the angle from the time. On zero, to SWCHCLR; otherwise, after the
 * EXIT, alarm 611. */
static const word p34_ttpi_words[] = {
    ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, ETPIFLAG),
    ADDRESS_PLUS_ONE(TTPI),
    STODL(TIG),
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(BZE, SET),
    ADDRESS(P34_P74B),
    SWITCH(SET, ETPIFLAG),
    /* P34/P74B */
    ORDER(CALL),
    ADDRESS(SELECTMU),
    /* P34/P74C */
    ORDERS(DLOAD, SET),
    ADDRESS_PLUS_ONE(ZEROVECS),
    SWITCH(SET, ITSWICH),
    ORDERS(BON, CLEAR),
    SWITCH(BON, ETPIFLAG),
    ADDRESS(SWCHSET),
    SWITCH(CLEAR, ITSWICH),
    /* SWCHSET */
    STORE(NOMTPI),
    /* INTLOOP */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(TTPI),
    ADDRESS_PLUS_ONE(NOMTPI),
    STCALL(TDEC1),
    ADDRESS(PRECSET),
    ORDER(CALL),
    ADDRESS(S33_34_1),
    ORDERS(BZE, EXIT),
    ADDRESS(SWCHCLR),
};

/* After the EXIT: alarm 611 (OCT 611 after the TC). */
void p34_alarm(void)
{
    tc_to(P34_P74E - 023u, ALARM);
}

/* and verb 05 noun 09 flashed (GOFLASH). */
void p34_after_alarm(void)
{
    set_A(V05N09);
    tc_to(P34_P74E - 020u, BANKCALL); /* CADR GOFLASH */
}

/* GOFLASH's terminate. */
void p34_terminate(void)
{
    tc_to(P34_P74E - 016u, GOTOPOOH);
}

/* "PROCEED": TTPI asked for again, at P34/P74A. */
void p34_proceed(void)
{
    tc_at(P34_P74E - 015u);
    p34_p74a();
}

/* "V32" (TC -7): the alarm again. */
void p34_recycle(void)
{
    tc_at(P34_P74E - 014u);
    p34_alarm();
}

/* SWCHCLR: with ITSWICH set (cleared by BONCLR), round INTLOOP again,
 * TTPI now NOMTPI on; with ETPIFLAG set, "DISPLAY TTPI" (P34/P74D). Or
 * else, after the EXIT, DISPLAYE. */
static const word p34_swchclr_words[] = {
    ORDERS(BONCLR, BON),   SWITCH(BONCLR, ITSWICH), ADDRESS(INTLOOP),
    SWITCH(BON, ETPIFLAG), ADDRESS(P34_P74D), /* "DISPLAY TTPI" */
    ORDER(EXIT),
};

/* After the EXIT: "DISPLAY ELEV AND CENTANG" (DISPLAYE). */
void swchclr_after_exit(void)
{
    tc_at(P34_P74E - 5u);
    displaye();
}

/* DISPLAYE's return: on at P34/P74E. */
void swchclr_after_displaye(void)
{
    tc_at(P34_P74E - 4u);
    p34_p74e();
}

/* P34/P74D: an EXIT, to show TTPI. */
static const word p34_p74d_words[] = {ORDER(EXIT)};

/* After it: "TTPI", verb 06 noun 37, through VNPOOH, which comes back to
 * P34/P74E. */
void p34_p74d_after_exit(void)
{
    set_A(V06N37);
    tc_at(P34_P74E - 1u);
    vnpooh();
}

/* P34/P74E: to the interpreter, for P34_P74E_CODE. */
void p34_p74e(void)
{
    tc_at(P34_P74E);
    intpret();
}

/* ---- The transfer -- LM pp. 662-663, CM pp. 464-465 ---- */

/* P34_P74E_CODE: X1 from RTX1 (the Earth's or the Moon's); the central
 * angle's cosine and sine (CSTH, SNTH); the passive vehicle's position
 * and velocity at TTPI (RPASS3, VPASS3), rescaled by X2, to RVEC and VVEC,
 * and RVSW set: TIMETHET finds the time T the passive vehicle takes
 * through the central angle. TTPI to INTIME, "FOR INITVEL", and TTPI plus
 * T, "RENDEZVOUS TIME", to TPASS4: S34/35.2 finds the velocity at TPI
 * that reaches the passive vehicle there. DELVTPI, the delta V at TPI
 * (DELVEET3's length); DELVTPF, the delta V at intercept (VPASS4 less
 * VTPRIME); POSTTPI, the perigee after TPI (PERIAPO1 from RACT3 and
 * VIPRIME, SHIFTR1); TTPI to TIG. Then, after the EXIT, verb 06 noun
 * 58. */
static const word p34_p74e_words[] = {
    ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RTX1),
    STODL(X1),
    ADDRESS_PLUS_ONE(CENTANG),
    ORDERS(PUSH, COS),
    STODL(CSTH),
    ORDER(SIN),
    STOVL(SNTH),
    ADDRESS_PLUS_ONE(RPASS3),
    ORDER(VSR_STAR),
    SR_X2(0),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(VPASS3),
    ORDERS(VSR_STAR, SET),
    SR_X2(0),
    SWITCH(SET, RVSW),
    STCALL(VVEC),
    ADDRESS(TIMETHET),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPI),
    STORE(INTIME), /* "FOR INITVEL" */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(T), /* "RENDEZVOUS TIME" */
    STCALL(TPASS4),      /* "FOR INITVEL" */
    ADDRESS(S34_35_2),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(DELVEET3),
    STOVL(DELVTPI),
    ADDRESS_PLUS_ONE(VPASS4),
    ORDERS(VSU, ABVAL),
    ADDRESS_PLUS_ONE(VTPRIME),
    STOVL(DELVTPF),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(VIPRIME),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    STODL(POSTTPI),
    ADDRESS_PLUS_ONE(TTPI),
    STORE(TIG),
    ORDER(EXIT),
};

/* After the EXIT: verb 06 noun 58 (POSTTPI, DELVTPI, DELVTPF), through
 * VNPOOH. */
void p34_p74e_after_exit(void)
{
    set_A(V06N58);
    tc_at(P34_P74E + 055u);
    vnpooh();
}

/* VNPOOH's return: to the interpreter, for P34_N58_CODE. */
void p34_p74e_after_v06n58(void)
{
    tc_at(P34_P74E + 056u);
    intpret();
}

/* P34_N58_CODE: S34/35.5, the delta V in local vertical coordinates for
 * the next display, and VN1645 (verb 16 noun 45, until the burn program
 * is chosen); then round again at P34/P74C. */
static const word p34_n58_words[] = {
    ORDER(CALL), ADDRESS(S34_35_5), ORDER(CALL), ADDRESS(VN1645), ORDER(GOTO), ADDRESS(P34_P74C),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P34_TTPI_CODE = {035u, TTPI_FIRST, COUNT(p34_ttpi_words),
                                                p34_ttpi_words};
const struct interpretive_code P34_SWCHCLR_CODE = {035u, P34_P74E - 013u, COUNT(p34_swchclr_words),
                                                   p34_swchclr_words};
const struct interpretive_code P34_P74D_CODE = {035u, P34_P74E - 3u, COUNT(p34_p74d_words),
                                                p34_p74d_words};
const struct interpretive_code P34_P74E_CODE = {035u, P34_P74E + 1u, COUNT(p34_p74e_words),
                                                p34_p74e_words};
const struct interpretive_code P34_N58_CODE = {035u, P34_P74E + 057u, COUNT(p34_n58_words),
                                               p34_n58_words};
#if !LUMINARY099
const struct interpretive_code P34_NN_CODE = {035u, P34_P74A + 4u, COUNT(p34_nn_words),
                                              p34_nn_words};
#endif
