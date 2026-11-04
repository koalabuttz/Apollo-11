/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 3 (Luminary099 pp. 678-681, Comanche055 pp.
 * 480-482): the displays at the end of each pass of the TPI and
 * mid-course programs. S34/35.5 shows the delta V in local vertical
 * coordinates (verb 06 noun 81), which the astronaut may replace with
 * one keyed in; then the delta V along the line of sight (verb 06 noun
 * 59).
 * VN1645 shows the marks, the time to go and the middle gimbal angle
 * (verb 16 noun 45) while the clock runs down (COMPTGO), until the
 * astronaut chooses: terminate (KILCLOCK), proceed (N45PROC: the next
 * pass the final one, or after it a new program), or recycle (CLUPDATE:
 * the next pass with the radar updates off). Their basic code, and their
 * interpretive code in the listing's notation (src/interpreter/
 * notation.h), which the readable interpreter runs (src/binding/
 * p34_routines.c). The faithful C is c64-port/src/lm_p34_p35.c and
 * cm_p34_p35.c.
 *
 * The missions differ in S34/35.5: the LM compares the delta V after the
 * display with a copy (GDT/2) word by word in basic code, and sets
 * NTARGFLG when the astronaut changed it; the CM keeps the copy in DVLOS
 * and compares the vectors in interpretive code. The LM flashes noun 81
 * through BANKCALL and GOFLASH itself, the CM through VNPOOH.
 *
 * Each call out of the file (BANKCALL, GOTOPOOH, UPFLAG, COMPTGO,
 * PHASCHNG) leaves Z at the routine and Q at the word after the TC;
 * VNPOOH and P3XORP7X, the file's own (displays.c), run on. */
#include "p34_p35.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))
#define FLAGON         IN_BANK(FLAGON)
#define FLAGOFF        IN_BANK(FLAGOFF)
#define GET45          IN_BANK(GET45)

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- S34/35.5 -- LM pp. 678-679, CM pp. 480-481 ---- */

/* S34/35.5: the return to SUBEXIT. With FINALFLG set, the displays
 * (FLAGON); else UPDATFLG set, and only noun 59 (FLAGOFF). FLAGON: the
 * delta V in local vertical coordinates, DELVLVC, kept (the LM's in
 * GDT/2, NTARGFLG clear; the CM's in DVLOS), and after the EXIT, noun 81
 * shown. */
static const word s34_35_5_words[] = {
    ORDERS(STQ, BON),     ADDRESS(SUBEXIT),        SWITCH(BON, FINALFLG),     ADDRESS(FLAGON),
    ORDERS(SET, GOTO),    SWITCH(SET, UPDATFLG),   ADDRESS(FLAGOFF),
/* FLAGON */
#if LUMINARY099
    ORDERS(CLEAR, VLOAD), SWITCH(CLEAR, NTARGFLG), ADDRESS_PLUS_ONE(DELVLVC), STORE(GDT_2),
#else
    ORDER(VLOAD),      ADDRESS_PLUS_ONE(DELVLVC), STORE(DVLOS), /* "SAVE DELTA V BEFORE DISPLAY" */
#endif
    ORDER(EXIT),
};

#if LUMINARY099
/* FLAGON +5: verb 06 noun 81 flashed (BANKCALL, CADR GOFLASH). */
void s34_35_5_after_exit(void)
{
    set_A(V06N81);
    tc_to(FLAGON + 6u, BANKCALL);
}

/* GOFLASH's returns: terminate (GOTOPOOH), */
void s34_35_5_terminate(void)
{
    tc_to(FLAGON + 010u, GOTOPOOH);
}

/* proceed ("PRO", TC +2: the comparison), */
void s34_35_5_proceed(void)
{
    tc_at(FLAGON + 011u);
    s34_35_5_compare();
}

/* and recycle ("LOAD", TC FLAGON +5: the display again, after the
 * astronaut's load). */
void s34_35_5_recycle(void)
{
    tc_at(FLAGON + 012u);
    s34_35_5_after_exit();
}

/* FLAGON +013: EBANK 7 ("TO BE SURE"), and the six words of DELVLVC
 * taken from GDT/2's, the differences summed in L (NTARGCHK, Q from 5
 * down to 0). Their sum not zero: the astronaut keyed in a new delta V,
 * NTARGFLG set (UPFLAG). Then to the interpreter. */
void s34_35_5_compare(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS); /* "TO BE SURE" */
    zl();
    set_A(FIVE);
    do {
        ts(Q_ADDRESS); /* NTARGCHK */
        cs(indexed(ADDRESS_OF(DELVLVC), Q));
        ad_at(indexed(ADDRESS_OF(GDT_2), Q));
        ads(L_ADDRESS);
    } while (ccs_at(Q_ADDRESS) == POSITIVE); /* CCS Q; TCF NTARGCHK */
    lxch(A_ADDRESS);
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        s34_35_5_after_upflag();
        return;
    }
    tc_to(FLAGON + 032u, UPFLAG); /* ADRES NTARGFLG */
}

/* FLAGON +034: to the interpreter, for S34_35_5_LOADED_CODE. */
void s34_35_5_after_upflag(void)
{
    tc_at(FLAGON + 034u);
    intpret();
}
#else
/* FLAGON +4: verb 06 noun 81 flashed (VNPOOH). */
void s34_35_5_after_exit(void)
{
    set_A(V06N81);
    tc_at(FLAGON + 5u);
    vnpooh();
}

/* FLAGON +6: to the interpreter, for S34_35_5_LOADED_CODE. */
void s34_35_5_after_vnpooh(void)
{
    tc_at(FLAGON + 6u);
    intpret();
}
#endif

/* S34_35_5_LOADED_CODE: a delta V the astronaut loaded (the LM's
 * NTARGFLG set; the CM's DELVLVC not DVLOS, "TEST FOR OVERWRITE OF
 * COMPUTED") makes the new target (S34/35.3). NOCHG: XDELVFLG clear, and
 * the delta V DELVEET3 to DELVSIN. FLAGOFF: S34/35.4, the delta V along
 * the line of sight, and after the EXIT, noun 59 shown. */
static const word s34_35_5_loaded_words[] = {
#if LUMINARY099
    ORDERS(BOFF, CALL),
    SWITCH(BOF, NTARGFLG),
    ADDRESS(NOCHG),
    ADDRESS(S34_35_3),
#else
    ORDERS(VLOAD, VSU), /* "TEST FOR OVERWRITE OF COMPUTED" */
    ADDRESS_PLUS_ONE(DELVLVC),
    ADDRESS_PLUS_ONE(DVLOS),
    ORDERS(ABVAL, BZE),
    ADDRESS(NOCHG), /* "NO OVERWRITE" */
    ORDER(CALL),
    ADDRESS(S34_35_3),
#endif
    /* NOCHG */
    ORDERS(CLEAR, VLOAD),
    SWITCH(CLEAR, XDELVFLG),
    ADDRESS_PLUS_ONE(DELVEET3),
    STORE(DELVSIN),
    /* FLAGOFF */
    ORDER(CALL),
    ADDRESS(S34_35_4),
    ORDER(EXIT),
};

/* FLAGOFF +3: verb 06 noun 59 (DVLOS) through VNPOOH. */
void flagoff_after_exit(void)
{
    set_A(V06N59);
    tc_at(FLAGOFF + 4u);
    vnpooh();
}

/* FLAGOFF +5: to the interpreter, and back by SUBEXIT. */
void flagoff_after_vnpooh(void)
{
    tc_at(FLAGOFF + 5u);
    intpret();
}

static const word flagoff_words[] = {ORDER(GOTO), ADDRESS(SUBEXIT)};

/* ---- VN1645 -- LM pp. 680-681, CM pp. 481-482 ---- */

/* VN1645: the return to SUBEXIT, and +MGA, the middle gimbal angle shown,
 * -.01 (DP-.01); with FINALFLG set, -.02. With FINALFLG and REFSMFLG set,
 * after the EXIT, the angle worked out (P3X) or left (P7X). */
static const word vn1645_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(SUBEXIT),
    ADDRESS_PLUS_ONE(DPM_01),
    STORE(PMGA), /* "MGA = -.01" */
    ORDERS(BOFF, DLOAD),
    SWITCH(BOF, FINALFLG),
    ADDRESS(GET45),
    ADDRESS_PLUS_ONE(DPM_01),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(DPM_01),
    STORE(PMGA), /* "MGA = -.02" */
    ORDERS(BOFF, EXIT),
    SWITCH(BOF, REFSMFLG),
    ADDRESS(GET45),
};

/* After the EXIT: P3XORP7X, which comes back to the TC +2 in P34 and
 * P35, to the TC GET45 +1 in P74 and P75. */
void vn1645_after_exit(void)
{
    tc_at(GET45 - 010u);
    p3xorp7x();
    if (Z == GET45 - 7u)
        vn1645_p3x();
    else
        vn1645_p7x();
}

/* "P3X": TC +2, the angle worked out; */
void vn1645_p3x(void)
{
    tc_at(GET45 - 7u);
    vn1645_mga();
}

/* "P7X": TC GET45 +1, the clock started. */
void vn1645_p7x(void)
{
    tc_at(GET45 - 6u);
    vn1645_clock();
}

/* GET45 -5: to the interpreter, for GETMGA_CODE. */
void vn1645_mga(void)
{
    tc_at(GET45 - 5u);
    intpret();
}

/* GETMGA_CODE: "COMPUTE MGA" from the delta V DELVSIN (GET+MGA). GET45:
 * the EXIT. */
static const word getmga_words[] = {
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(DELVSIN),
    ORDER(CALL), /* "COMPUTE MGA" */
    ADDRESS(GETPMGA),
    /* GET45 */
    ORDER(EXIT),
};

/* GET45 +1: "INITIATE TASK TO UPDATE TTOGO" (COMPTGO). */
void vn1645_clock(void)
{
    tc_to(GET45 + 1u, COMPTGO);
}

/* GET45 +2: SUBEXIT kept in QSAVED (TS skips the CAF on an overflow), and
 * a second's wait (BANKCALL, CADR DELAYJOB). */
void vn1645_after_comptgo(void)
{
    ca(ADDRESS_OF(SUBEXIT));
    if (!ts(ADDRESS_OF(QSAVED))) set_A(ONE_SEC);
    tc_to(GET45 + 5u, BANKCALL);
}

/* GET45 +7: verb 16 noun 45 flashed, "TRKMKCNT, TTOGO, +MGA" (BANKCALL,
 * CADR GOFLASH). */
void vn1645_after_delayjob(void)
{
    set_A(V16N45);
    tc_to(GET45 + 010u, BANKCALL);
}

/* GOFLASH's returns: "TERMINATE" (KILCLOCK), */
void vn1645_terminate(void)
{
    tc_at(GET45 + 012u);
    kilclock();
}

/* "PROCEED" (N45PROC), */
void vn1645_proceed(void)
{
    tc_at(GET45 + 013u);
    n45proc();
}

/* and "RECYCLE -- RETURN FOR INITIAL COMPUTATION" (CLUPDATE). */
void vn1645_recycle(void)
{
    tc_at(GET45 + 014u);
    clupdate();
}

/* KILCLOCK: DISPDEX positive (CA Z: the address after the CA), which
 * stops the clock, and the program ended (GOTOPOOH). */
void kilclock(void)
{
    set_A(KILCLOCK + 1u); /* CA Z */
    ts(ADDRESS_OF(DISPDEX));
    tc_to(KILCLOCK + 2u, GOTOPOOH);
}

/* N45PROC: with FINALFLG set, "FLASH V37 -- AWAIT NEW PGM" (KILCLOCK);
 * else, after a restart point (PHASCHNG, OCT 04024), FINALFLG set. */
void n45proc(void)
{
    cs(ADDRESS_OF(FLAGWRD2));
    mask(BIT6);
    if (A_is_zero()) { /* EXTEND; BZF KILCLOCK */
        kilclock();
        return;
    }
    tc_to(N45PROC + 4u, PHASCHNG);
}

void n45proc_after_phaschng(void)
{
    tc_to(N45PROC + 6u, UPFLAG); /* ADRES FINALFLG */
}

/* CLUPDATE: the clock stopped (DISPDEX), a restart point, and to the
 * interpreter. */
void clupdate(void)
{
    set_A(CLUPDATE + 1u); /* CA Z */
    ts(ADDRESS_OF(DISPDEX));
    tc_to(CLUPDATE + 2u, PHASCHNG);
}

void clupdate_after_phaschng(void)
{
    tc_at(CLUPDATE + 4u);
    intpret();
}

/* CLUPDATE_CODE: UPDATFLG clear, and back by QSAVED, for the next
 * pass. */
static const word clupdate_words[] = {
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, UPDATFLG),
    ADDRESS(QSAVED),
};

/* ---- The interpretive code ---- */

#if LUMINARY099
#define LOADED_FIRST (FLAGON + 035u)
#else
#define LOADED_FIRST (FLAGON + 7u)
#endif

const struct interpretive_code S34_35_5_CODE = {035u, IN_BANK(S34_35_5), COUNT(s34_35_5_words),
                                                s34_35_5_words};
const struct interpretive_code S34_35_5_LOADED_CODE = {
    035u, LOADED_FIRST, COUNT(s34_35_5_loaded_words), s34_35_5_loaded_words};
const struct interpretive_code FLAGOFF_CODE = {035u, FLAGOFF + 6u, COUNT(flagoff_words),
                                               flagoff_words};
const struct interpretive_code VN1645_CODE = {035u, IN_BANK(VN1645), COUNT(vn1645_words),
                                              vn1645_words};
const struct interpretive_code GETMGA_CODE = {035u, GET45 - 4u, COUNT(getmga_words), getmga_words};
const struct interpretive_code CLUPDATE_CODE = {035u, CLUPDATE + 5u, COUNT(clupdate_words),
                                                clupdate_words};
