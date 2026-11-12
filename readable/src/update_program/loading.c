/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, part 2: UPPART2, loading UPBUFF from the DSKY
 * (OHWELL1, OHWELL2) and the ground's check of it (UPVERIFY), in bank 04
 * (the CM's in bank 27); LM pages 1389-1391, CM pages 1500-1502. The
 * faithful C is c64-port/src/lm_update.c and cm_update.c. */
#include "update_program.h"

/* CK4V32 -- LM p. 1391, CM p. 1502, "ON DATA RETURN FROM 'GOXDSPF'", with
 * the verb in MPAC: "SO TEST FOR V32." True on data ("IT'S
 * NOT A V32, IT'S DATA.  PROCEED.": TC Q, back to the word after the
 * caller's TC); false on V32 ("GO BACK AND GET DATA": INDEX Q; TC 0 -6,
 * to the request six words before that word). CS MPAC; MASK BIT6 leaves
 * BIT6 or +0, never a sign for the CCS's other two ways. */
static bool ck4v32(void)
{
    cs(ADDRESS_OF(MPAC));
    mask(BIT6);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        return_through_q();
        return true;
    }
    tc_at((word)(CK4V32 + 5u)); /* TC 0 -6 */
    return false;
}

/* The display requests. "DISPLAY A FLASHING V21N01" (UPLOADNV) or V21N02
 * (UPVRFYNV) by GOXDSPF, which comes back to one of the three words after
 * its CADR: V34, V33, or data (or V32). */
static void flash(word verb_noun, word at)
{
    set_A(verb_noun);
    tc_to(at, BANKCALL); /* CADR GOXDSPF */
}

/* OHWELL1 +2: "REQUEST USER TO SEND NUMBER ... OF COMPONENTS
 * PARAMETER(II)." MPAC +2 holds the ECADR where PINBALL puts it, UPBUFF. */
static void ohwell1_request(void)
{
    flash(UPLOADNV, (word)(OHWELL1 + 3u));
}

/* OHWELL2 +3: "REQUEST DATA." into the ECADR at MPAC +2. */
static void ohwell2_request(void)
{
    flash(UPLOADNV, (word)(OHWELL2 + 4u));
}

/* OHWELL2 +2, with A an ECADR in UPBUFF: "PLACE ECADR INTO R3.", and ask
 * for the component. */
static void ohwell2_at(void)
{
    ts_no_overflow(ADDRESS_OF(MPAC) + 2u);
    ohwell2_request();
}

/* OHWELL2 -1 and OHWELL2 -- LM p. 1390, CM p. 1501: "INCREMENT COUNT OF
 * COMPONENTS RECEIVED.", and "CALCULATE LOCATION(ECADR) IN UPBUFF WHERE
 * NEXT COMPONENT SHOULD BE STORED." (UPBUFF -1 plus UPCOUNT). */
static void ohwell2(void)
{
    set_A(ADUPBFM1);
    ad_at(ADDRESS_OF(UPCOUNT));
    ohwell2_at();
}

static void ohwell2_next(void)
{
    incr(ADDRESS_OF(UPCOUNT));
    ohwell2();
}

/* UPVERIFY -- LM p. 1390, CM p. 1502: "PLACE ECADR WHERE COMPONENT NO.
 * INDEX IS TO BE STORED INTO R3." (UPTEMP), and "DISPLAY A FLASHING V21N02
 * TO REQUEST DATA CORRECTION OR VERIFICATION." */
static void upverify_request(void)
{
    flash(UPVRFYNV, (word)(UPVERIFY + 3u));
}

static void upverify(void)
{
    set_A(ADUPTEMP);
    ts(ADDRESS_OF(MPAC) + 2u);
    upverify_request();
}

/* UPPART2 -- LM p. 1389, CM p. 1500: "SET RESTART GROUP 6 TO RESTORE OLD
 * MODE AND DOWNLIST AND EXIT IF RESTART OCCURS." (PHASCHNG, to UPOUT +1),
 * at a "PRIORITY SAME AS CHRPRIO". */
void uppart2(void)
{
    tc_to(UPPART2, PHASCHNG); /* OCT 07026; OCT 30000; 2CADR UPOUT +1 */
}

/* UPPART2 +5, where PHASCHNG returns: DNLSTCOD 1 (the "DOWNLIST"), and
 * "SET MAJOR MODE = 27". */
void uppart2_after_phaschng(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(DNLSTCOD));
    tc_to((word)(UPPART2 + 7u), NEWMODEX); /* DEC 27 */
}

/* UPPART2 +011, where NEWMODEX returns. INDEX UPVERB; TCF +1: "BRANCH
 * DEPENDING ON WHETHER THE UPDATE VERB REQUIRES A FIXED OR VARIABLE"
 * number of components. V71 and V72 "GO GET NO. OF COMPONENTS" (OHWELL1);
 * V70 and V73 set COMPNUMB "TO 2." and "GO GET THE TWO UPDATE COMPONENTS"
 * (OHWELL2). Not modelled: an UPVERB that makes the TCF any other. */
void uppart2_after_newmodex(void)
{
    word fan = (word)(UPPART2 + 013u); /* where the TCF +1 goes */
    word to = indexed_instruction(TCF_TO(fan), fetch(ADDRESS_OF(UPVERB)));

    if (to == TCF_TO(fan + 1u) || to == TCF_TO(fan + 2u)) { /* V71, V72 */
        set_A(ADUPBUFF);                                    /* OHWELL1 */
        ts(ADDRESS_OF(MPAC) + 2u);
        ohwell1_request();
        return;
    }
    if (to != TCF_TO(fan) && to != TCF_TO(fan + 3u)) not_modelled();
    set_A(TWO); /* "V73 (AND V70) FIXED" */
    ts(ADDRESS_OF(COMPNUMB));
    set_A(ADUPBFM1); /* TCF OHWELL2 */
    ad_at(ADDRESS_OF(UPCOUNT));
    ohwell2_at();
}

/* OHWELL1 +5: "V34 TERMINATE UPDATE(P27) RETURN" (UPOUT4). */
void ohwell1_terminate(void)
{
    upout4();
}

/* OHWELL1 +6: V33 asks again. */
void ohwell1_proceed(void)
{
    ohwell1_request();
}

/* OHWELL1 +7: "DATA OR V32 RETURN". "IS II(NUMBER OF COMPONENTS
 * PARAMETER) .GE. 3 AND .LE. 20D." (UP21 is 21: "MAX NO OF COMPONENTS
 * +1"). If not, ask again; if so, "SAVE II IN COMPNUMB", and on to the
 * first component. */
void ohwell1_data(void)
{
    tc_at((word)(OHWELL1 + 7u)); /* TC CK4V32 */
    if (!ck4v32()) {
        ohwell1_request();
        return;
    }
    set_A(negate(BIT2));
    ad_at(ADDRESS_OF(UPBUFF));
    if (A_is_zero_or_minus()) {
        ohwell1_request();
        return;
    }
    cs(ADDRESS_OF(UPBUFF));
    ad(UP21);
    if (A_is_zero_or_minus()) {
        ohwell1_request();
        return;
    }
    ca(ADDRESS_OF(UPBUFF));
    ts(ADDRESS_OF(COMPNUMB));
    ohwell2_next();
}

/* OHWELL2 +6: "V34 TERMINATE UPDATE(P27) RETURN." */
void ohwell2_terminate(void)
{
    upout4();
}

/* OHWELL2 +7: "V33 PROCEED RETURN", asking for the component again. */
void ohwell2_proceed(void)
{
    ohwell2_request();
}

/* OHWELL2 +010: "DATA OR V32 RETURN". "HAVE WE FINISHED RECEIVING ALL THE
 * DATA WE EXPECTED.": "YES- GO TO VERIFICATION SEQUENCE", "NO- REQUEST
 * ADDITIONAL DATA." */
void ohwell2_data(void)
{
    tc_at((word)(OHWELL2 + 010u)); /* TC CK4V32 */
    if (!ck4v32()) {
        ohwell2_request();
        return;
    }
    cs(ADDRESS_OF(UPCOUNT));
    ad_at(ADDRESS_OF(COMPNUMB));
    if (A_is_zero_or_minus())
        upverify();
    else
        ohwell2_next();
}

/* UPVERIFY +5: "V34 TERMINATE UPDATE(P27) RETURN" */
void upverify_terminate(void)
{
    upout4();
}

/* UPVERIFY +6: "V33 DATA SENT IS GOOD. GO STORE IT." */
void upverify_proceed(void)
{
    upstore();
}

/* UPVERIFY +7: "COMPONENT NO. INDEX OR V32 RETURN". "DOES THE COMPONENT
 * NO. INDEX JUST SENT SPECIFY A LEGAL COMPONENT NUMBER?" Not if "IT IS NOT
 * POSITIVE NONZERO", nor above COMPNUMB: verify again. If so, "BASED ON
 * THE COMPONENT NO. INDEX CALCULATE THE ECADR OF LOCATION IN UPBUFF WHICH
 * USER WANTS TO CHANGE." and ask for it (OHWELL2 +2). */
void upverify_data(void)
{
    tc_at((word)(UPVERIFY + 7u)); /* TC CK4V32 */
    if (!ck4v32()) {
        upverify_request(); /* UPVERIFY +2 */
        return;
    }
    ca(ADDRESS_OF(UPTEMP));
    if (A_is_zero_or_minus()) {
        upverify();
        return;
    }
    cs(ADDRESS_OF(UPTEMP));
    ad_at(ADDRESS_OF(COMPNUMB));
    ad(BIT1);
    if (A_is_zero_or_minus()) {
        upverify();
        return;
    }
    set_A(ADUPBFM1);
    ad_at(ADDRESS_OF(UPTEMP));
    ohwell2_at();
}
