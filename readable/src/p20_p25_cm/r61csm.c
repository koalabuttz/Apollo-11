/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 4 (Comanche055 pp. 574-575): R61CSM, the "PREFERRED
 * TRACKING ATTITUDE ROUTINE" (bank 37).
 *
 * P20's tracking calls R61CSM by BANKCALL, again and again. It works out
 * the CSM's attitude for tracking the LM with the optics (CRS61.1) and, when
 * the attitude must change, maneuvers to it by R60CSM. R61CNTR paces it:
 * positive, R61CSM counts it down and leaves ("NOT READY TO DO R61"); +0 or
 * -0, it runs; negative, a maneuver (KALCMANU) is under way and it leaves.
 * After a run that needs no maneuver R61CNTR is 3, after a maneuver +0.
 * GENRET keeps the return (by MAKECADR) and SAVBNK the caller's EBANK, and
 * DECRM61 goes back through BANKJUMP.
 *
 * Each call out of the file (MAKECADR, BANKCALL, UPFLAG, DOWNFLAG,
 * PHASCHNG) leaves Z at the routine and Q at the word after the TC, where
 * the routine comes back to; BANKJUMP goes back to R61CSM's caller. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r61csm_not_ready(void);
static void decrm61(void);
static void decrm61_plus1(void);

/* R61CSM -- p. 574. "SWITCH TO EBANK 6" and "SAVE EBANK" (the caller's,
 * which XCH leaves in A); the return as a CADR (MAKECADR). (A bank
 * register's word leaves no overflow for the TS to skip on.) */
void r61csm(void)
{
    set_A(EBANK6);
    xch(EBANK_ADDRESS);
    if (ts(ADDRESS_OF(SAVBNK))) not_modelled();
    tc_to(R61CSM + 3u, MAKECADR);
}

/* R61CSM +4: the return to GENRET, and R61CNTR tested: positive, "NOT
 * READY TO DO R61"; +0, "DO R61"; negative, out; -0, R61 too. (MAKECADR's
 * CADR leaves no overflow; were there one, the TS would skip the CCS to
 * the TC DECRM61.) */
void r61csm_after_makecadr(void)
{
    if (ts(ADDRESS_OF(GENRET))) {
        r61csm_not_ready();
        return;
    }
    switch (ccs_at(ADDRESS_OF(R61CNTR))) {
    case POSITIVE: r61csm_not_ready(); break;
    case IS_PLUS_ZERO: r61csm_do_r61(); break;
    case NEGATIVE: r61csm_leave(); break;
    case IS_MINUS_ZERO: r61csm_interpret(); break;
    }
}

/* R61CSM +6: TC DECRM61, A (after the CCS, R61CNTR less one) to R61CNTR. */
static void r61csm_not_ready(void)
{
    tc_at(R61CSM + 6u);
    decrm61();
}

/* R61CSM +7: "DO R61": TC +2. */
void r61csm_do_r61(void)
{
    tc_at(R61CSM + 7u);
    r61csm_interpret();
}

/* R61CSM +010: TC DECRM61 +1, out with R61CNTR as it is. */
void r61csm_leave(void)
{
    tc_at(R61CSM + 010u);
    decrm61_plus1();
}

/* R61CSM +011: CRS61.1, in interpretive code. */
void r61csm_interpret(void)
{
    tc_at(R61CSM + 011u);
    intpret();
}

/* R61CSM +012: CRS61.1, the "LOS DETERMINATION + VEH ATTITUDE", leaves
 * in MPAC what to do. */
static const word r61csm_words[] = {
    ORDER(CALL),
    ADDRESS(CRS61_1), /* "LOS DETERMINATION + VEH ATTITUDE" */
    ORDER(EXIT),
};

/* R61CSM +015 -- after the EXIT: INDEX MPAC; TC +1. MPAC 0 runs the TC at
 * R61CSM +017 to R61END: no maneuver (a "SUBROUTINE DRIVING DAP", "AUTO
 * MODE NOT SET"); 1 the word after it, R61C1, the maneuver. Not modelled:
 * any other MPAC, which would run a word past them as an instruction. */
void r61csm_after_exit(void)
{
    word to = indexed_instruction((word)(R61CSM + 017u), fetch(ADDRESS_OF(MPAC)));

    tc_at(R61CSM + 016u);
    if (to == R61CSM + 017u)
        r61csm_no_maneuver();
    else if (to == R61C1)
        r61c1();
    else
        not_modelled();
}

/* R61CSM +017: TC R61END. */
void r61csm_no_maneuver(void)
{
    tc_at(R61CSM + 017u);
    r61end();
}

/* R61C1: "RESET 3-AXIS FLAG" ("BIT 6 FLAG 5"). */
void r61c1(void)
{
    tc_to(R61C1, DOWNFLAG); /* ADRES 3AXISFLG */
}

/* R61C1 +2 -- p. 575: "SET R61CNTR NEG. TO INDICATE KALCMANU"; "SET FLAG
 * FOR PRIORITY DISPLAYS FOR R60". (CS ONE leaves no overflow for the TS to
 * skip on.) */
void r61c1_after_3axisflg(void)
{
    set_A(negate(ONE)); /* CS ONE */
    if (ts(ADDRESS_OF(R61CNTR))) not_modelled();
    tc_to(R61C1 + 4u, UPFLAG); /* ADRES PDSPFLAG */
}

/* R61C1 +6: the maneuver, R60CSM. */
void r61c1_after_pdspflag(void)
{
    tc_to(R61C1 + 6u, BANKCALL); /* CADR R60CSM */
}

/* R61C1 +010: "RESET FLAG FOR PRIORITY DISPLAYS IN R60". */
void r61c1_after_r60csm(void)
{
    tc_to(R61C1 + 010u, DOWNFLAG); /* ADRES PDSPFLAG */
}

/* R61C1 +012: a restart phase change (PHASCHNG, OCT 00111), */
void r61c1_after_pdspflag_reset(void)
{
    tc_to(R61C1 + 012u, PHASCHNG);
}

/* R61C1 +014: and R61CNTR +0, the next pass's R61 to run. */
void r61c1_after_phaschng(void)
{
    set_A(ZERO);
    tc_at(R61C1 + 015u);
    decrm61();
}

/* R61END: R61CNTR 3, three passes before the next R61. */
void r61end(void)
{
    set_A(THREE);
    decrm61();
}

/* DECRM61: A to R61CNTR, and back: "RETURN IS IN L", "RESTORE EBANK",
 * "RETURN IS NOW BACK IN A", and "EXIT R61" by BANKJUMP. (No A that comes
 * here overflows, for the TS to skip on.) */
static void decrm61(void)
{
    if (ts(ADDRESS_OF(R61CNTR))) not_modelled();
    decrm61_plus1();
}

static void decrm61_plus1(void)
{
    ca(ADDRESS_OF(GENRET)); /* CAE GENRET */
    lxch(A_ADDRESS);
    ca(ADDRESS_OF(SAVBNK));
    xch(EBANK_ADDRESS);
    lxch(A_ADDRESS);
    tc_to(DECRM61 + 6u, BANKJUMP);
}

/* ---- The interpretive code ---- */

const struct interpretive_code R61CSM_CODE = {037u, R61CSM + 012u, COUNT(r61csm_words),
                                              r61csm_words};

#endif
