/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 16 (Comanche055 pp. 622-624): P23's marks and
 * POINTAXS (bank 31).
 *
 * P23.60, after R52's marks: MARKSTAT's low ten bits are the mark data's
 * address (MARKDATA); its first two words, the mark's time, go to
 * MARKTIME, its word 5 to TRUNION, and words 0 to 6 to MARKDOWN for the
 * downlink. Verb 05 noun 71 flashes for the crew: terminate, proceed
 * (P23.65, "STORE DATA": SAVECFLG set, and the codes checked again from
 * P23.15, part 15), or enter (the display again). With SAVECFLG set,
 * P23.31 goes to P23.85: RENDWFLG cleared, POINTAXS, and R23.55, the
 * measurement's incorporation (part 17).
 *
 * POINTAXS: the "VECTOR FROM S/C TO HORIZON OR LANDMARK". With ORBWFLAG
 * clear (the W matrix not valid), INITIALW sets it up ("INITIALIZE
 * W-MATRIX FIRST PASS IN P23") from WMIDPOS; with it set, DIM0FLAG too
 * (the W matrix to be used). INTEGRV brings the CSM's state vector
 * to the mark's time, and RECT.1 picks it up. When the planet measured is
 * not the primary body (ZMEASURE), the CSM's position (RZC) is shifted by
 * the moon's (LUNPOS): added, or its complement in the earth's sphere.
 * Then the landmark's position (LALOTORV, ERADFLAG set) or, with no
 * landmark, the horizon's (HORIZ, part 18), less RZC, returned through
 * POINTEX.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, GOTOPOOH) leaves Z at
 * the routine and Q at the word after the TC, where the routine comes
 * back to (after its data word, for PHASCHNG); GOTOPOOH doesn't come
 * back. GOFLASH comes back to the word after its CADR on terminate, the
 * next on proceed, and the next on enter. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* P23.60 +1 -- after the EXIT: with interrupts inhibited, the mark data's
 * address (MASK LOW10) to MARKDATA; INDEX MARKDATA, DCA 0 and DXCH: its
 * first two words to MARKTIME; INDEX MARKDATA, CA 5 and XCH: its word 5
 * to TRUNION. Not modelled: mark data that runs into fixed memory
 * (MARKDATA 01773 or more). (A MASK leaves no overflow for a TS to skip
 * on, and an index under 02000 leaves the DCA and the CA as they are.) */
void p23_60_after_exit(void)
{
    word data;

    inhint();
    ca(ADDRESS_OF(MARKSTAT));
    mask(LOW10);
    if (ts(ADDRESS_OF(MARKDATA))) not_modelled();
    data = fetch(ADDRESS_OF(MARKDATA));
    if (data + 5u >= 02000u) not_modelled();
    dca(data); /* EXTEND; INDEX MARKDATA; DCA 0 */
    dxch(ADDRESS_OF(MARKTIME));
    ca(data + 5u); /* INDEX MARKDATA; CA 5 */
    xch(ADDRESS_OF(TRUNION));
    relint();
    tc_at(P23_60 + 015u);
    intpret();
}

/* P23.60 +016: the mark data to MARKDOWN for the downlink. X1 is
 * MARKDATA's complement (LXC,1), so that n,1 is the mark data's word n:
 * words 1 to 6 as a vector to MARKDOWN +1, and words 0 and 1 as a double
 * to MARKDOWN. */
static const word p23_60_words[] = {
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(MARKDATA),
    ADDRESS_PLUS_ONE_X1(1),
    STODL_STAR_AT(MARKDOWN, 1),
    ADDRESS_PLUS_ONE_X1(0),
    STORE(MARKDOWN),
    ORDER(EXIT),
};

/* P23.60 +025 -- after the EXIT: verb 05 noun 71 flashed by GOFLASH. */
void p23_60_display(void)
{
    set_A(V05N71);
    tc_to(P23_60 + 026u, BANKCALL); /* CADR GOFLASH */
}

/* P23.60 +030: "TERMINATE". */
void p23_60_terminate(void)
{
    tc_to(P23_60 + 030u, GOTOPOOH);
}

/* P23.60 +031: "STORE DATA": TC P23.65. */
void p23_60_proceed(void)
{
    tc_at(P23_60 + 031u);
    p23_65();
}

/* P23.60 +032: "REDISPLAY": TC -5. */
void p23_60_redisplay(void)
{
    tc_at(P23_60 + 032u);
    p23_60_display();
}

/* P23.65: TC INTPRET, */
void p23_65(void)
{
    tc_at(P23_65);
    intpret();
}

/* P23.65 +1: SAVECFLG set, for P23.31. */
static const word p23_65_words[] = {
    ORDERS(SET, EXIT),
    SWITCH(SET, SAVECFLG),
};

/* P23.65 +3 -- after the EXIT: TC P23.15, the codes checked again. */
void p23_65_after_exit(void)
{
    tc_at(P23_65 + 3u);
    p23_15();
}

/* P23.85, which P23.31 goes to with SAVECFLG set: RENDWFLG cleared (for
 * P20), the line of sight by POINTAXS, and R23.55. */
static const word p23_85_words[] = {
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, RENDWFLG),
    ADDRESS(POINTAXS),
    ORDER(GOTO),
    ADDRESS(R23_55),
    /* POINTAXS: "WE BEGIN CALCULATIONS HERE" ("POINTAXIS SUBROUTINE"), the
     * return in POINTEX. */
    ORDER(STQ),
    ADDRESS(POINTEX),
    /* R23.05: with ORBWFLAG set, on to R23.1. */
    ORDERS(BON, DLOAD),
    SWITCH(BON, ORBWFLAG),
    ADDRESS(R23_1),
    ADDRESS_PLUS_ONE(WMIDPOS),
    STCALL_D(0),
    /* "INITIALIZE W-MATRIX FIRST PASS IN P23" */
    ADDRESS(INITIALW),
    /* R23.1 */
    ORDER(CALL),
    /* "SETUP FOR CSM INTEGRATION" */
    ADDRESS(SETINTG),
    ORDERS(BOF, SET),
    SWITCH(BOF, ORBWFLAG),
    ADDRESS(R23_2),
    SWITCH(SET, DIM0FLAG),
    /* R23.2 */
    ORDERS(SET, CALL),
    SWITCH(SET, ORBWFLAG),
    /* "INTEGRATE CSM STATE VEC. TO MARKTIME" */
    ADDRESS(INTEGRV),
    ORDER(EXIT),
};

/* R23.2 +4 -- after the EXIT: a restart point. */
void r23_2_after_exit(void)
{
    tc_to(R23_2 + 4u, PHASCHNG); /* OCT 04022 */
}

/* R23.2 +6: TC INTPRET, */
void r23_2_after_phaschng(void)
{
    tc_at(R23_2 + 6u);
    intpret();
}

/* R23.2 +7: "PICKUP CSM STATE VECTOR FROM PERM"; "IN SPHERE OF INFLUENCE
 * OF PRIMARY BODY" (ZMEASURE clear), on to R23.3; else RZC shifted by
 * the moon's position at the mark's time (LUNPOS), complemented unless
 * CMOONFLG is set. Then the landmark's position, or the horizon's ("IF
 * LANDMARK = 0, USE HORIZ SUBR"), less RZC, back through POINTEX with
 * the push-down pointer 0. */
static const word r23_2_words[] = {
    ORDER(CALL),
    /* "PICKUP CSM STATE VECTOR FROM PERM" */
    ADDRESS(RECT_1),
    ORDER(BOFF),
    /* "IN SPHERE OF INFLUENCE OF PRIMARY BODY" */
    SWITCH(BOF, ZMEASURE),
    ADDRESS(R23_3),
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(MARKTIME),
    ADDRESS(LUNPOS),
    ORDERS(BON, VCOMP),
    SWITCH(BON, CMOONFLG),
    ADDRESS_AT(R23_3, -3), /* "+1": past the VCOMP */
    ORDER(VAD),
    ADDRESS_PLUS_ONE(RZC),
    STORE(RZC),
    /* R23.3 */
    ORDERS(SLOAD, BHIZ),
    /* "IF LANDMARK = 0, USE HORIZ SUBR" */
    ADDRESS_PLUS_ONE(LANDMARK),
    ADDRESS(R23_4),
    ORDER(SET),
    SWITCH(SET, ERADFLAG),
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(MARKTIME),
    ADDRESS(LALOTORV),
    ORDER(GOTO),
    ADDRESS(R23_5),
    /* R23.4 */
    ORDER(CALL),
    ADDRESS(HORIZ),
    /* R23.5 */
    ORDERS(VSU, SETPD),
    ADDRESS_PLUS_ONE(RZC),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(GOTO),
    ADDRESS(POINTEX),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P23_60_CODE = {031u, P23_60 + 016u, COUNT(p23_60_words),
                                              p23_60_words};
const struct interpretive_code P23_65_CODE = {031u, P23_65 + 1u, COUNT(p23_65_words), p23_65_words};
const struct interpretive_code P23_85_CODE = {031u, P23_85, COUNT(p23_85_words), p23_85_words};
const struct interpretive_code R23_2_CODE = {031u, R23_2 + 7u, COUNT(r23_2_words), r23_2_words};

#endif
