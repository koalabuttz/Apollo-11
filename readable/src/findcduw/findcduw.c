/* SPDX-License-Identifier: MIT */
/* FINDCDUW--GUIDAP_INTERFACE.agc, pp. 910-912 (Luminary099, bank 30):
 * INITCDUW, FINDCDUW's housekeeping, its basic data and inputs (PAUTNO),
 * and the thrust direction filter. The faithful C is
 * c64-port/src/lm_findcduw.c. */
#include "findcduw.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- INITCDUW -- p. 910 ---- */

/* INITCDUW: "A SINGLE INTERPRETIVE CALL TO INITCDUW IS REQUIRED BEFORE
 * EACH GUIDED MANEUVER USING FINDCDUW." The filtered thrust direction
 * UNFV/2 and the window command UNWC/2 both start as the X axis, and back
 * (RVQ). */
static const word initcduw_words[] = {
    /* INITCDUW */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNITX),
    STORE(UNFV_2),
    STORE(UNWC_2),
    ORDER(RVQ),
};

const struct interpretive_code INITCDUW_CODE = {030u, INITCDUW, COUNT(initcduw_words),
                                                initcduw_words};

/* ---- FINDCDUW -- p. 910 ---- */

/* FINDCDUW -2: "ENTRY WHEN UNFC/2 PRE-STORD"; FINDCDUW: "ENTRY WHEN
 * UNFC/2 IN MPAC". "INPUT VECTORS NEED NOT BE SEMI-UNIT". BOV clears the
 * overflow indicator ("INTERPRETER NOW INITIALIZED"), and SETPD 22 (octal)
 * keeps the push-down list's "LOCS 0 THRU 21 FOR DIRECTION COSINE MAT"; the
 * return goes to QCDUWUSR (STQ), and on in basic code (EXIT). */
static const word findcduw_words[] = {
    /* FINDCDUW -2 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNFC_2),
    /* FINDCDUW */ ORDERS(BOV, SETPD),
    ADDRESS(FINDCDUW),
    ADDRESS_PLUS_ONE_D(022), /* "22", octal */
    ORDERS(STQ, EXIT),
    ADDRESS(QCDUWUSR), /* "SAVE RETURN ADDRESS" */
};

const struct interpretive_code FINDCDUW_CODE = {030u, FINDCDUW - 2u, COUNT(findcduw_words),
                                                findcduw_words};

/* "MORE HAUSKEEPING": FINDCDUW's E-bank ("SET EBANK"), the caller's kept
 * in ECDUWUSR. NDXCDUW, the index of the constants' pairs, is 1 "IF CSM
 * DOCKED", else 0; FLPAUTNO is XOVINHIB's bit, "POS-NON-ZERO FLAG PNGCS
 * AUTO NOT", until the test below clears it; FLAGOODW, the window good
 * flag, is the bit if the X axis override is inhibited, else +0. (After
 * each MASK, A is +0 or the bit: CCS takes its first two ways only. A is
 * a word at each TS here: none skips.)
 *
 * "FETCH BASIC DATA": with interrupts inhibited ("RELINT AT PAUTNO (TC
 * INTPRET)"), CDUSPOT gets the three CDUs, Y, Z, X (CDUSPOTX is CDUSPOT
 * +4), or in PNGCS auto the commanded ones, CDUXD-CDUZD. Channel 30's bit
 * 10 and channel 31's bit 14 are set when not in PNGCS and not in auto
 * ("BITS INVERTED"); RAND leaves +0 or the bit. */
void findcduw_after_exit(void)
{
    set_A(ECDUWL);
    xch(EBANK_ADDRESS);
    ts(ADDRESS_OF(ECDUWUSR));
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD); /* "CSMDOCKD MUST NOT BE BIT15" */
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(ONE);
    ts(ADDRESS_OF(NDXCDUW));
    set_A(XOVINHIB);
    ts(ADDRESS_OF(FLPAUTNO));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(FLAGOODW));

    inhint();
    ca(ADDRESS_OF(CDUX)); /* "FETCH CDUX,CDUY,CDUZ IN ALL CASES" */
    ts(ADDRESS_OF(CDUSPOTX));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUSPOTY));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUSPOTZ));
    set_A(BIT10); /* "PNGCS CONTROL BIT" */
    rand_channel(030u);
    if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) {
        set_A(BIT14); /* "AUTO MODE BIT" */
        rand_channel(031u);
        if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) {
            ts(ADDRESS_OF(FLPAUTNO)); /* "RESET FLAG PNGCS AUTO NOT" */
            ca(ADDRESS_OF(CDUXD));    /* "PNGCS AUTO: FETCH CDUXD,CDUYD,CDUZD" */
            ts(ADDRESS_OF(CDUSPOTX));
            ca(ADDRESS_OF(CDUYD));
            ts(ADDRESS_OF(CDUSPOTY));
            ca(ADDRESS_OF(CDUZD));
            ts(ADDRESS_OF(CDUSPOTZ));
        }
    }
    tc_at(PAUTNO); /* PAUTNO: TC INTPRET */
    intpret();
}

/* ---- PAUTNO -- p. 912 ---- */

/* "FETCH INPUTS": the thrust command (in MPAC) made semi-unit (NORMUNIT),
 * "AS INITIAL UNX/2"; the window command too, "AS INITIAL UNZ/2", and
 * QUICTRIG, "ALWAYS RQD TO OBTAIN TRIGS OF CDUD'S" (the angles in
 * CDUSPOT). A command too short to make a unit vector ("AT LEAST ONE
 * ENTERING CMD VCT ZERO") goes to NOATTCNT (BOVB). Then DELV made a unit
 * vector: "IF UNIT DELV OVERFLOWS SKIP FILTER" (BOV AFTRFLTR); else
 * *SMNB* "YIELDS UNIT(DELV) IN VEH COORDS FOR FLTR", and the basic code
 * (EXIT). */
static const word pautno_words[] = {
    /* PAUTNO +1 */ ORDER(RTB),
    ADDRESS(NORMUNIT),
    STOVL(UNX_2),
    ADDRESS_PLUS_ONE(UNWC_2),
    ORDERS(RTB, RTB),
    ADDRESS(NORMUNIT),
    ADDRESS(QUICTRIG),
    STOVL(UNZ_2),
    ADDRESS_PLUS_ONE(DELV),
    ORDERS(BOVB, UNIT),
    ADDRESS(NOATTCNT),
    ORDERS(BOV, CALL),
    ADDRESS(AFTRFLTR),
    ADDRESS(STAR_SMNB_STAR),
    ORDER(EXIT),
};

const struct interpretive_code PAUTNO_CODE = {030u, PAUTNO + 1u, COUNT(pautno_words), pautno_words};

/* ---- The thrust direction filter -- p. 912 ---- */

/* "THRUST DIRECTION FILTER": UNFV/2, the thrust direction in vehicle
 * axes, follows the measured one in MPAC, Y then Z, through FLTRSUB.
 * "FOR RESTARTS, UNFV/2 ALWAYS INTACT, MPAC RENEWD AFTER RETURN FROM
 * CALLER, TWO FILTER UPDATES MAY BE DONE. UNFV/2 NEED NOT BE EXACTLY
 * SEMI-UNIT." FLTRSUB returns through TEM2 (TC TEM2), which leaves the
 * file: each return is a key of its own. */
void thrust_filter(void)
{
    ca(ADDRESS_OF(UNFVY_2));
    lxch(MPAC_AT(3u));
    tc_at(PAUTNO + 022u);
    fltrsub();
}

/* FLTRSUB's return: the new Y offset, and Z. (An A that overflows skips
 * the CA: FLTRSUB then filters UNFVY/2's +1 or -1.) */
void thrust_filter_after_y(void)
{
    if (!ts(ADDRESS_OF(UNFVY_2))) ca(ADDRESS_OF(UNFVZ_2));
    lxch(MPAC_AT(5u));
    tc_at(PAUTNO + 026u);
    fltrsub();
}

/* FLTRSUB's return: the new Z offset, and the interpreter again
 * ("COMPLETES FILTER"), at AFTRFLTR. Not modelled: an A that overflows,
 * whose TS would skip the TC INTPRET into the interpretive word after it.
 */
void thrust_filter_after_z(void)
{
    if (ts(ADDRESS_OF(UNFVZ_2))) not_modelled();
    tc_at(AFTRFLTR - 1u);
    intpret();
}

#endif
