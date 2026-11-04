/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 6 (Luminary099 pp. 700-702, Comanche055 pp.
 * 501-504): R36, the out-of-plane rendezvous routine, "TO DISPLAY AT
 * ASTRONAUT REQUEST LGC CALCULATED RENDEZVOUS OUT-OF-PLANE PARAMETERS (Y,
 * YDOT, PSI)." (verb 90). Its basic code, and its interpretive code in the
 * listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/p34_routines.c). The faithful C is
 * c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * Verb 06 noun 16 asks for the time of the event (zero for now). Both
 * vehicles are integrated to it: the first one's orbit gives the plane,
 * and the second one's position and velocity give Y, its distance from
 * the plane (RANGE), YDOT, its rate (RRATE), and PSI, the angle from its
 * forward horizontal to the line of sight in the horizontal plane
 * (RTHETA). Verb 06 noun 90 shows them, and the time again.
 *
 * The missions differ: the LM's is in bank 4 and always shows the LM out
 * of the CSM's plane. The CM's first asks which vehicle (R36, bank 20:
 * verb 04 noun 12, option 2, "R2 LOADED VIA DSKY": 1 for the CSM, 2 for
 * the LM), keeps the answer in OPTIONY, and integrates the vehicles in
 * that order; its code is spread over banks 40, 41 and 42.
 *
 * Each call out of the file (BANKCALL, ENDEXT, POSTJUMP) leaves Z at the
 * routine and Q at the word after a TC. */
#include "p34_p35.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))
#define R36TAG2        IN_BANK(R36TAG2)

/* The banks: the LM's all in 4; the CM's display code in 41, R36INT in
 * 42 and R36B in 40 (the superbank's, FB 30-32 with the superbank bit). */
#if LUMINARY099
#define BASIC_BANK  04u
#define R36INT_BANK 04u
#define R36B_BANK   04u
#else
#define BASIC_BANK  041u
#define R36INT_BANK 042u
#define R36B_BANK   040u
#endif

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

#if COMANCHE055
/* ---- The CM's R36 -- CM p. 502, bank 20 ---- */

/* R36: verb 04 noun 12 (OPTION36) with option 2 and, in R2, 1 (the CSM),
 * for the astronaut to change (BANKCALL, CADR GOXDSPF). TS skips the next
 * CAF on an overflow, which a CAF never leaves. */
void r36(void)
{
    set_A(TWO);
    if (!ts(ADDRESS_OF(OPTIONX))) set_A(ONE);
    if (!ts(ADDRESS_OF(OPTIONX) + 1u)) set_A(OPTION36);
    tc_to(R36 + 5u, BANKCALL);
}

/* GOXDSPF's returns: "TERMINATE" (ENDEXT), */
void r36_terminate(void)
{
    tc_to(R36 + 7u, ENDEXT);
}

/* "PROCEED" (TC +2, to R36A by POSTJUMP), */
void r36_proceed(void)
{
    tc_at(R36 + 010u);
    r36_to_r36a();
}

/* and "R2 LOADED VIA DSKY" (TC -5: the display again). */
void r36_loaded(void)
{
    tc_at(R36 + 011u);
    set_A(OPTION36);
    tc_to(R36 + 5u, BANKCALL);
}

/* TC POSTJUMP, CADR R36A: on in bank 41. */
void r36_to_r36a(void)
{
    tc_to(R36 + 012u, POSTJUMP);
}
#endif

/* ---- R36 -- LM p. 701, CM p. 502 ---- */

/* The CM's R36A, the LM's R36: L and the time (DSPTEMX) +0, "SET TIME OF
 * EVENT TO ZERO FOR FIRST DISPLAY". The CM's DSPTEMX is OPTIONX, so the
 * DXCH also brings the option's R2 into L: "SAVE VEH. OPTION" in OPTIONY. */
void r36_start(void)
{
    zl();
    set_A(ZERO);
    dxch(ADDRESS_OF(DSPTEMX));
#if COMANCHE055
    lxch(ADDRESS_OF(OPTIONY));
#endif
    r36_ask_time();
}

/* The LM's R36 +3, the CM's R36P3: verb 06 noun 16 flashed for the time
 * (BANKCALL, CADR GOMARKF). */
void r36_ask_time(void)
{
#if LUMINARY099
    word at = R36 + 3u;
#else
    word at = R36P3;
#endif
    set_A(V06N16N);
    tc_to(at + 1u, BANKCALL);
}

/* GOMARKF's returns: "TERMINATE" (ENDEXT), */
void r36_time_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* "PROCEED": the time keyed in, double precision from DSPTEMX (DXCH),
 * and if A is zero the low word decides (LREGCHK); a time other than zero
 * to MPAC (ASTROTIM), and to the interpreter. */
void r36_time_proceed(void)
{
    dxch(ADDRESS_OF(DSPTEMX)); /* TCF +2 */
    if (A_is_zero()) {         /* EXTEND; BZF LREGCHK */
        lregchk();
        return;
    }
    astrotim();
}

/* and "RECYCLE FOR ASTRONAUT INPUT TIME" (TCF -5). */
void r36_time_recycle(void)
{
    r36_ask_time();
}

/* ASTROTIM: "TIME = ASTRO INPUT TIME", to MPAC, and TC INTPRET. */
void astrotim(void)
{
    dxch(ADDRESS_OF(MPAC));
    tc_at(ASTROTIM + 1u);
    intpret();
}

/* LREGCHK: the low word (XCH L) zero too: "SET TIME = PRESENT TIME"
 * (ENTTIM2, to the interpreter); else the time back as it was (XCH L)
 * and on at ASTROTIM. */
void lregchk(void)
{
    xch(L_ADDRESS);
    if (A_is_zero()) { /* EXTEND; BZF ENTTIM2 */
        tc_at(ENTTIM2);
        intpret();
        return;
    }
    xch(L_ADDRESS);
    astrotim(); /* TCF ASTROTIM */
}

/* ASTROTIM +2: the time made a double precision value (RTB DPMODE), and
 * on at R36INT (the LM's next, the CM's by GOTO). ENTTIM2 +1: the time
 * now (RTB LOADTIME) instead. */
static const word astrotim_words[] = {
#if LUMINARY099
    ORDER(RTB),
    ADDRESS(DPMODE),
#else
    ORDERS(RTB, GOTO),
    ADDRESS(DPMODE),
    ADDRESS(R36INT),
#endif
};
static const word enttim2_words[] = {
    ORDERS(RTB, GOTO),
    ADDRESS(LOADTIME),
    ADDRESS(R36INT),
};

/* R36INT: the first vehicle at the time (TDEC1): the LM's the other one
 * (OTHPREC, the CSM); the CM's by OPTIONY, shifted right one place (zero
 * for 1, R36PROG2: the LM, OTHPREC; else the CSM, THISPREC). R36PROG3:
 * its position, RPASS36, and the unit normal of its orbit, UNP36 (from
 * its velocity and unit position, through STADR). The time (TAT, as the
 * integration left it) again for the second vehicle: the LM's itself
 * (THISPREC); the CM's the other of the two (R36PROG4, R36PROG5).
 *
 * Its velocity (0D) and position, and the time kept in 30D "FOR
 * REDISPLAY"; the position twice (6D, 12D), the line of sight from it to
 * the first vehicle's (12D); Y = UNP36 . R (RANGE) and its rate, YDOT =
 * UNP36 . V (RRATE); 18D, the position's unit vector, and from the
 * velocity the unit horizontal ahead. */
static const word r36int_words[] = {
#if LUMINARY099
    STCALL(TDEC1),
    ADDRESS(OTHPREC),
#else
    STORE(TDEC1),
    ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(OPTIONY),
    ORDERS(BHIZ, CALL),
    ADDRESS(R36PROG2), /* "FOR CSM DISPLAY" */
    ADDRESS(THISPREC), /* "FOR LEM DISPLAY" */
    ORDER(GOTO),
    ADDRESS(R36PROG3),
    /* R36PROG2 */
    ORDER(CALL),
    ADDRESS(OTHPREC),
/* R36PROG3 */
#endif
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RPASS36),
    ORDERS(UNIT, PDVL),
    ORDERS(VXV, UNIT),
    ORDER(STADR),
    AFTER_STADR(STODL(UNP36)),
    ADDRESS_PLUS_ONE(TAT),
#if LUMINARY099
    STCALL(TDEC1),
    ADDRESS(THISPREC),
#else
    STORE(TDEC1),
    ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(OPTIONY),
    ORDERS(BHIZ, CALL),
    ADDRESS(R36PROG4), /* "FOR CSM DISPLAY" */
    ADDRESS(OTHPREC),  /* "FOR LEM DISPLAY" */
    ORDER(GOTO),
    ADDRESS(R36PROG5),
    /* R36PROG4 */
    ORDER(CALL),
    ADDRESS(THISPREC),
/* R36PROG5 */
#endif
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(VATT), /* "VELOCITY VECTOR" */
    ADDRESS_PLUS_ONE(RATT),
    ORDER(PDDL),
    ADDRESS_PLUS_ONE(TAT), /* "SAVE TIME IN LOCATION 30D FOR REDISPLAY" */
    STOVL_D(30),
    ORDERS(PUSH, PUSH), /* "POSITION VECTOR" */
    ORDERS(BVSU, PDVL),
    ADDRESS_PLUS_ONE(RPASS36), /* "LINE OF SIGHT VECTOR" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UNP36),
    STOVL(RANGE),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UNP36),
    STOVL(RRATE),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(UNIT, PUSH),
    ORDERS(VXV, VXV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(VSL2, UNIT),
#if LUMINARY099
    ORDER(UNIT),
#else
    ORDERS(UNIT, GOTO),
    ADDRESS(R36B),
#endif
};

/* The CM's R36B: the "UNIT HORIZONTAL IN FORWARD DIR." in 0D; the line of
 * sight less its vertical part, "LOS PROJECTED INTO HORIZONTAL" plane
 * (12D), and PSI the angle between them (RTHETA); when the line of sight
 * is to the other side (the sign of their cross product along the unit
 * position), PSI is a revolution less it (LODPMAX, the CM's DPPOSMAX). */
static const word r36b_words[] = {
    STOVL_D(0), /* "UNIT HORIZONTAL IN FORWARD DIR. 00D" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(DOT, VXSC),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(VSL2),
    ORDERS(BVSU, UNIT),
    ORDER(UNIT),
    ORDERS(PUSH, DOT), /* "LOS PROJECTED INTO HORIZONTAL" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SL1, ARCCOS),
    STOVL(RTHETA),
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(BPL, DLOAD),
    ADDRESS(R36TAG2),
#if LUMINARY099
    ADDRESS_PLUS_ONE(LODPMAX),
#else
    ADDRESS_PLUS_ONE(DPPOSMAX),
#endif
    ORDER(DSU),
    ADDRESS_PLUS_ONE(RTHETA),
#if LUMINARY099
    STORE(RTHETA),
#else
    STCALL(RTHETA),
    ADDRESS(R36TAG2),
#endif
};

/* R36TAG2: the time from 30D with its two words' signs agreeing
 * (SGNAGREE) to DSPTEMX, for the display; then out of the interpreter. */
static const word r36tag2_words[] = {
    ORDERS(DLOAD, RTB), ADDRESS_PLUS_ONE_D(30), ADDRESS(SGNAGREE), STORE(DSPTEMX), ORDER(EXIT),
};

/* After the EXIT: "DISPLAY Y, YDOT, AND PSI." (verb 06 noun 90, BANKCALL,
 * CADR GOMARKF). */
void r36_after_exit(void)
{
    set_A(V06N90N);
    tc_to(R36TAG2 + 6u, BANKCALL);
}

/* GOMARKF's returns: "TERMINATE" and "PROCEED, END OF PROGRAM" (ENDEXT),
 * and "REDISPLAY OUTPUT" (the time asked for again). */
void r36_end_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

void r36_end_proceed(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

void r36_redisplay(void)
{
    r36_ask_time();
}

/* ---- The interpretive code ---- */

#if LUMINARY099
/* The LM's R36B part, just before R36TAG2. */
#define R36B_FIRST ((word)(R36TAG2 - COUNT(r36b_words)))
#else
#define R36B_FIRST IN_BANK(R36B)
#endif

const struct interpretive_code ASTROTIM_CODE = {BASIC_BANK, ASTROTIM + 2u, COUNT(astrotim_words),
                                                astrotim_words};
const struct interpretive_code ENTTIM2_CODE = {BASIC_BANK, ENTTIM2 + 1u, COUNT(enttim2_words),
                                               enttim2_words};
const struct interpretive_code R36INT_CODE = {R36INT_BANK, IN_BANK(R36INT), COUNT(r36int_words),
                                              r36int_words};
const struct interpretive_code R36B_CODE = {R36B_BANK, R36B_FIRST, COUNT(r36b_words), r36b_words};
const struct interpretive_code R36TAG2_CODE = {BASIC_BANK, R36TAG2, COUNT(r36tag2_words),
                                               r36tag2_words};
