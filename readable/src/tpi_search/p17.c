/* SPDX-License-Identifier: MIT */
/* TPI_SEARCH.agc of the CM, part 3 (Comanche055 pp. 559-561, bank 35):
 * the "TPI SEARCH DISPLAY ROUTNE", P17 and P77, which run the search
 * (S17.1 and S17.2) between the crew's displays.
 *
 * P17 searches with the CSM as the active vehicle, P77 with the LM
 * (AVFLAGA, AVFLAGP); both set the track and update flags and go on at
 * P17.1:
 *   P17.1 shows the TPI time TTPI (verb 06 noun 37, VNPOOH), brings both
 *   vehicles to it (S17.1) and picks the search option K from the sign
 *   of the altitude difference: 2 with KFLAG clear, 1 with it set. It
 *   shows the phase angle, the altitude difference and K (verb 06 noun
 *   72); K as the crew leaves it sets KFLAG again ("RESET KFLAG ON FOR
 *   OPTION =1", "OFF FOR OPTION =2"). The safe pericenter HPERMIN is the
 *   Moon's (HPL, "35000FT") with X2 in XRS +1 set, else the Earth's
 *   (HPE, "85 N.M."); and P17.2 searches (S17.2).
 *   P17.3 shows the result, "DELTA VTPI , DELTA VTPF , AND H" (verb 06
 *   noun 58), then the "PERICENTER CODE AND CENTRAL ANG." (verb 06 noun
 *   55, flashed by GOFLASHR): terminate or proceed ends the program,
 *   recycle goes back to P17.1, and the immediate return blanks R2 and
 *   ends the job.
 *   ALARUMS, where S17.2 goes when "NO SAFE PERICENTER IN THIS SECTOR",
 *   raises alarm 124 and shows it (verb 05 noun 09); proceed ends the
 *   program.
 *
 * VNCOMP17 flashes each display (GOFLASH), its return kept in QSAVED and
 * its verb and noun in VERBNOUN: terminate shows it again ("TERMINATE
 * ILLEGAL REDISPLAY"), proceed returns through QSAVED, and recycle
 * (enter) goes back to P17.1 with MPAC equal to BIT6 ("RECYCLE WITH NEW
 * TPI TIME") and shows the display again with anything else ("OR PROCEED
 * WITH NEW SEARCH OPTION").
 *
 * Each call out of the file (AVFLAGA, AVFLAGP, P20FLGON, VNPOOH, BANKCALL,
 * BLANKET, ALARM) leaves Z at the routine and Q at the word after the TC
 * (after its CADR or OCT), where the routine comes back to; ENDOFJOB and
 * GOTOPOOH don't come back. GOFLASHR and GOFLASH come back to the word
 * after the CADR on terminate, the next on proceed, the next on recycle
 * and, for GOFLASHR, the next on its immediate return. */
#include "tpi_search.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

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

static void vncomp17(void);

/* ---- P17 and P77 -- p. 559 ---- */

/* P17: "AVFLAG = CSM , SET TRACK + UPDATE FLAGS", */
void p17(void)
{
    tc_to(P17, AVFLAGA);
}

/* P17 +1: and on at P17.1. */
void p17_after_avflaga(void)
{
    tc_at(P17 + 1u);
    p17_1();
}

/* P77: "AVFLAG = LEM , SET TRACK + UPDATE FLAGS", and on at P17.1 (AVFLAGP
 * comes back to it). */
void p77(void)
{
    tc_to(P77, AVFLAGP);
}

/* ---- P17.1 -- pp. 559-560 ---- */

/* P17.1: "SET UPDATE FLAG" (P20FLGON, which sets the track flag too), */
void p17_1(void)
{
    tc_to(P17_1, P20FLGON);
}

/* P17.1 +1: "DISPLAY TTPI TIME", verb 06 noun 37 (VNPOOH), */
void p17_1_after_p20flgon(void)
{
    set_A(V06N37);
    tc_to(P17_1 + 2u, VNPOOH);
}

/* P17.1 +3: and to the interpreter, for P17_1_CODE. */
void p17_1_after_vnpooh(void)
{
    tc_at(P17_1 + 3u);
    intpret();
}

/* P17.1 +4: S17.1 with UPDATFLG clear ("UPDATE STATE VECTORS TO TTPI"),
 * then UPDATFLG set again; the search option to OPTION2: 2 with KFLAG
 * clear (the passive above the active), 1 with it set (BOFF +2 skips the
 * AXT,1 1). Out of the interpreter. */
static const word p17_1_words[] = {
    ORDERS(CLEAR, CALL), SWITCH(CLEAR, UPDATFLG), ADDRESS(S17_1),
    ORDERS(SET, AXT_1),  SWITCH(SET, UPDATFLG),   NUMBER(2),
    ORDERS(BOFF, AXT_1), SWITCH(BOFF, KFLAG),     ADDRESS_AT(P17_1, 016), /* +2 */
    NUMBER(1),           ORDERS(SXA_1, EXIT),     ADDRESS(OPTION2),
};

/* P17.1 +020, after the EXIT: "DISPLAY PHI , DELTA H , SEARCH OPTION K",
 * verb 06 noun 72 (VNCOMP17), */
void p17_1_after_exit(void)
{
    set_A(V06N72);
    tc_at(P17_1 + 021u);
    vncomp17();
}

/* P17.1 +022: and to the interpreter, for P17_1_OPTION_CODE. */
void p17_1_after_vncomp17(void)
{
    tc_at(P17_1 + 022u);
    intpret();
}

/* P17.1 +023: UPDATFLG clear and KFLAG set; KFLAG cleared unless OPTION2
 * less P21ONENN (1) is zero (BHIZ +2 skips the CLEAR); HPERMIN the Moon's
 * HPL with X2 (XRS +1) set, else (BHIZ +4) the Earth's HPE. P17.2: the
 * search, S17.2 (which goes to ALARUMS on its error exit); then UPDATFLG
 * set again, and out of the interpreter to P17.3. */
static const word p17_1_option_words[] = {
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, UPDATFLG),
    SWITCH(SET, KFLAG),
    ORDERS(SLOAD, DSU),
    ADDRESS_PLUS_ONE(OPTION2),  /* "RESET KFLAG ON FOR OPTION =1" */
    ADDRESS_PLUS_ONE(P21ONENN), /* "OFF FOR OPTION =2" */
    ORDERS(BHIZ, CLEAR),
    ADDRESS_AT(P17_1, 034), /* +2 */
    SWITCH(CLEAR, KFLAG),
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE_AT(XRS, 1),
    ADDRESS_AT(P17_1, 042), /* +4 */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(HPL),
    ADDRESS(P17_2),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HPE),
    /* P17.2 */ STCALL(HPERMIN),
    ADDRESS(S17_2),
    ORDERS(SET, EXIT),
    SWITCH(SET, UPDATFLG),
};

/* ---- P17.3 -- p. 560 ---- */

/* P17.3, after the EXIT: "DISPLAY DELTA VTPI , DELTA VTPF , AND H", verb
 * 06 noun 58 (VNCOMP17), */
void p17_3(void)
{
    set_A(V06N58);
    tc_at(P17_3 + 1u);
    vncomp17();
}

/* P17.3 +2: then "DISPLAY PERICENTER CODE AND CENTRAL ANG.", verb 06 noun
 * 55, flashed by GOFLASHR. */
void p17_3_after_vncomp17(void)
{
    set_A(V06N55);
    tc_to(P17_3 + 3u, BANKCALL); /* CADR GOFLASHR */
}

/* P17.3 +5: terminate, "TERMINATE PROGRAM". */
void p17_3_terminate(void)
{
    tc_to(P17_3 + 5u, GOTOPOOH);
}

/* P17.3 +6: proceed, "END PROGRAM". */
void p17_3_proceed(void)
{
    tc_to(P17_3 + 6u, GOTOPOOH);
}

/* P17.3 +7: recycle, "RECYCLE WITH NEW TTPI OR SEARCH OPTION". */
void p17_3_recycle(void)
{
    tc_at(P17_3 + 7u);
    p17_1();
}

/* P17.3 +010: the immediate return: "BLANK R2" (BLANKET, A two), */
void p17_3_immediate_return(void)
{
    set_A(TWO);
    tc_to(P17_3 + 011u, BLANKET);
}

/* P17.3 +012: and the job ends. */
void p17_3_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF */
}

/* ---- VNCOMP17 -- pp. 560-561 ---- */

/* VNCOMP17 +3 on: the display in VERBNOUN, flashed by GOFLASH (TCR). */
static void vncomp17_flash(void)
{
    ca(ADDRESS_OF(VERBNOUN));
    tc_to(VNCOMP17 + 4u, BANKCALL); /* CADR GOFLASH */
}

/* VNCOMP17: the return in QSAVED (EXTEND; QXCH), the verb and noun in A
 * to VERBNOUN (TS: on an overflow in A the TS skips the CA, and A, +1 or
 * -1, goes to GOFLASH), and the display. */
static void vncomp17(void)
{
    qxch(ADDRESS_OF(QSAVED));
    if (!ts(ADDRESS_OF(VERBNOUN))) ca(ADDRESS_OF(VERBNOUN));
    tc_to(VNCOMP17 + 4u, BANKCALL); /* CADR GOFLASH */
}

/* VNCOMP17 +6: terminate, "TERMINATE ILLEGAL REDISPLAY" (TC -3). */
void vncomp17_terminate(void)
{
    tc_at(VNCOMP17 + 6u);
    vncomp17_flash();
}

/* VNCOMP17 +7: proceed ("PROCEED"), back through QSAVED. */
void vncomp17_proceed(void)
{
    tc_at(VNCOMP17 + 7u);
    tc_return_in(ADDRESS_OF(QSAVED));
}

/* VNCOMP17 +010: recycle: MPAC equal to BIT6 (-MPAC + BIT6 +0 or -0,
 * EXTEND; BZF) goes back to P17.1, "RECYCLE WITH NEW TPI TIME"; anything
 * else shows the display again (TC VNCOMP17 +3), "OR PROCEED WITH NEW
 * SEARCH OPTION". */
void vncomp17_recycle(void)
{
    cs(ADDRESS_OF(MPAC));
    ad(BIT6);
    if (A_is_zero()) {
        p17_1();
        return;
    }
    tc_at(VNCOMP17 + 014u);
    vncomp17_flash();
}

/* ---- ALARUMS -- p. 561 ---- */

/* ALARUMS: UPDATFLG set, and out of the interpreter. */
static const word alarums_words[] = {
    ORDERS(SET, EXIT),
    SWITCH(SET, UPDATFLG),
};

/* ALARUMS +2, after the EXIT: alarm 124 (ALARM, OCT 00124), "NO SAFE
 * PERICENTER IN THIS SECTOR", */
void alarums_after_exit(void)
{
    tc_to(ALARUMS + 2u, ALARM); /* OCT 00124 */
}

/* ALARUMS +4: shown, verb 05 noun 09 (VNCOMP17), */
void alarums_after_alarm(void)
{
    set_A(V05N09);
    tc_at(ALARUMS + 5u);
    vncomp17();
}

/* ALARUMS +6: and on proceed, "PROCEED ILLEGAL TERMINATE PROGRAM". */
void alarums_after_vncomp17(void)
{
    tc_to(ALARUMS + 6u, GOTOPOOH);
}

/* ---- The interpretive code ---- */

const struct interpretive_code P17_1_CODE = {035u, P17_1 + 4u, COUNT(p17_1_words), p17_1_words};
const struct interpretive_code P17_1_OPTION_CODE = {035u, P17_1 + 023u, COUNT(p17_1_option_words),
                                                    p17_1_option_words};
const struct interpretive_code ALARUMS_CODE = {035u, ALARUMS, COUNT(alarums_words), alarums_words};

#endif
