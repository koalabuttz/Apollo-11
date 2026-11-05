/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 869-872 (Luminary099 only, banks 37, 33 and 21): the
 * PIPA reader, PIPASR, "TRYING TO BE VERY CAREFUL SO THAT WILL BE
 * RESTARTABLE", and its restart, REREADAC; TMPTOSPT, the CDUs of the
 * reading into CDUSPOT; and LRHTASK, the landing radar's altitude task.
 * The faithful C is c64-port/src/lm_servicer.c.
 *
 * Each PIPA counter is read and set to -0 in one exchange (DXCH PIPAX,
 * XCH PIPAZ), with its complement kept in TEMX, TEMY or TEMZ first: those
 * start at -0, and DELVX, DELVY and DELVZ at +0. So after a restart
 * REREADAC can tell, from which of them have changed, how far the reading
 * got. */
#include "servicer.h"

#if LUMINARY099

/* ---- PIPASR -- pp. 869-870 (bank 37) ---- */

/* PIPASR: the time of the reading (TIME2, TIME1) into PIPTIME1, the
 * "CURRENT TIME POSITIVE VALUE", ... */
void pipasr(void)
{
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(PIPTIME1));
    pipasr_plus_3();
}

/* ... (PIPASR +3) TEMX, TEMY and TEMZ "INITIALIZE ... AT NEG. ZERO.", DELV
 * +0 (DELVX's high word is written by the reading), and PIPAGE +0: "SHOW
 * PIPA READING IN PROGRESS"; ... */
void pipasr_plus_3(void)
{
    set_A(negate(ZERO));
    ts(ADDRESS_OF(TEMX));
    ts(ADDRESS_OF(TEMY));
    ts(ADDRESS_OF(TEMZ));

    set_A(ZERO);
    ts(ADDRESS_OF(DELVZ));
    ts(ADDRESS_OF(DELVZ) + 1u);
    ts(ADDRESS_OF(DELVY));
    ts(ADDRESS_OF(DELVY) + 1u);
    ts(ADDRESS_OF(DELVX) + 1u);
    ts(ADDRESS_OF(PIPAGE));
    repip1();
}

/* ... REPIP1: "X AND Y PIPS READ": their complements into TEMX and TEMY,
 * then the counts out and -0 in ("PIPAS SET TO NEG ZERO AS READ."), into
 * DELVX and DELVY; ... */
void repip1(void)
{
    dcs(ADDRESS_OF(PIPAX));
    dxch(ADDRESS_OF(TEMX));
    dxch(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(DELVX));
    lxch(ADDRESS_OF(DELVY));
    repip3();
}

/* ... REPIP3: "REPEAT PROCESS FOR Z PIP", ... */
void repip3(void)
{
    cs(ADDRESS_OF(PIPAZ));
    xch(ADDRESS_OF(TEMZ));
    xch(ADDRESS_OF(PIPAZ));
    dodelvz();
}

/* ... DODELVZ: into DELVZ. (Not modelled: entered with an overflow in A,
 * the TS would skip REPIP4's EXTEND and run its DCA as a CA.) */
void dodelvz(void)
{
    if (ts(ADDRESS_OF(DELVZ))) not_modelled();
    repip4();
}

/* REPIP4: "COMPUTE GUIDANCE PERIOD" (PGUIDE, PIPTIME1 less the last
 * reading's PIPTIME), the CDUs into CDUTEMP, "READ CDUS INTO HIGH ORDER
 * CDUTEMPS", and the counts into PIPATMP. Back (TC Q). */
void repip4(void)
{
    dca(ADDRESS_OF(PIPTIME1));
    dxch(ADDRESS_OF(PGUIDE));
    dcs(ADDRESS_OF(PIPTIME));
    das(ADDRESS_OF(PGUIDE));

    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUTEMPX));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUTEMPY));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUTEMPZ));
    ca(ADDRESS_OF(DELVX));
    ts(ADDRESS_OF(PIPATMPX));
    ca(ADDRESS_OF(DELVY));
    ts(ADDRESS_OF(PIPATMPY));
    ca(ADDRESS_OF(DELVZ));
    ts(ADDRESS_OF(PIPATMPZ));
    return_to_q();
}

/* ---- REREADAC -- p. 871 (bank 37) ---- */

/* REREADAC: with PIPAGE positive, "PIP READING NOT STARTED.  GO TO
 * BEGINNING" (READACCS). Else the reading goes on where it stopped, its
 * return PIPSDONE ("SET UP RETURN FROM PIPASR": DONEADR's GENADR in Q).
 * (CCS PIPAGE's other outcomes, as the AGC runs them: negative, Q gets
 * PIPAGE's magnitude less one; -0, Q is left as it is.) */
void rereadac(void)
{
    switch (ccs_at(ADDRESS_OF(PIPAGE))) {
    case POSITIVE: readaccs(); return;
    case IS_PLUS_ZERO:
        set_A(DONEADR);
        ts(Q_ADDRESS);
        break;
    case NEGATIVE: ts(Q_ADDRESS); break;
    case IS_MINUS_ZERO: break;
    }
    /* DELVZ +0: "Z NOT DONE, CHECK Y."; anything else, "Z DONE, GO DO
     * CDUS" (REPIP4). */
    if (ccs_at(ADDRESS_OF(DELVZ)) != IS_PLUS_ZERO) {
        repip4();
        return;
    }
    /* DELVY +0: "Y NOT DONE, CHECK X." (CHKTEMX, L +0). Else "Y DONE, ZERO
     * Z PIP.", its count to L. */
    zl();
    if (ccs_at(ADDRESS_OF(DELVY)) == IS_PLUS_ZERO) {
        chktemx();
        return;
    }
    lxch(ADDRESS_OF(PIPAZ));
    /* "TEMZ NOT = -0, CONTAINS -PIPAZ VALUE.": the count is its
     * complement (CS TEMZ; but +0 for TEMZ +0, which the CCS leaves in A),
     * into DELVZ. "TEMZ = -0, L HAS ZPIP VALUE.": that, and REPIP4. */
    switch (ccs_at(ADDRESS_OF(TEMZ))) {
    case POSITIVE:
    case NEGATIVE: cs(ADDRESS_OF(TEMZ)); break;
    case IS_PLUS_ZERO: break;
    case IS_MINUS_ZERO:
        lxch(ADDRESS_OF(DELVZ));
        repip4();
        return;
    }
    dodelvz();
}

/* CHKTEMX: TEMX -0, unchanged: X and Y not read (REPIP1). Else ("YES")
 * their counts are the complements of TEMX and TEMY (+0 for TEMX +0, as
 * the CCS leaves it), into DELVX and DELVY; "ZERO X AND Y PIPS" (-0 and
 * the +0 left in L: "L STILL ZERO FROM ABOVE"), and Z (REPIP3). */
void chktemx(void)
{
    switch (ccs_at(ADDRESS_OF(TEMX))) { /* "HAS THIS CHANGED" */
    case POSITIVE:
    case NEGATIVE: cs(ADDRESS_OF(TEMX)); break;
    case IS_PLUS_ZERO: break;
    case IS_MINUS_ZERO: repip1(); return; /* "NO" */
    }
    ts(ADDRESS_OF(DELVX));
    cs(ADDRESS_OF(TEMY));
    ts(ADDRESS_OF(DELVY));
    set_A(negate(ZERO));
    dxch(ADDRESS_OF(PIPAX));
    repip3();
}

/* ---- TMPTOSPT -- p. 872 (bank 33) ---- */

/* TMPTOSPT: the CDUs of the PIPA reading (CDUTEMP) into CDUSPOT, in its
 * order Y, Z, X. Back (TC Q). */
void tmptospt(void)
{
    ca(ADDRESS_OF(CDUTEMPY));
    ts(ADDRESS_OF(CDUSPOTY));
    ca(ADDRESS_OF(CDUTEMPZ));
    ts(ADDRESS_OF(CDUSPOTZ));
    ca(ADDRESS_OF(CDUTEMPX));
    ts(ADDRESS_OF(CDUSPOTX));
    return_to_q();
}

/* ---- LRHTASK -- p. 872 (bank 21) ---- */

/* LRHTASK, "A WAITLIST TASK SET BY READACCS DURING THE DESCENT BRAKING
 * PHASE": unless "LR BYPASS SET -- BYPASS ALL LR READING." (LRBYPASS),
 * with the READLR flag set and the LR read not inhibited (NOLRREAD
 * clear), the LRHJOB job (NOVAC at priority 32, 2CADR LRHJOB): "LR READ
 * OK.  SET JOB TO DO IT ABOUT 50 MS. PRIOR TO PIPA READ."; then GRP2OFF. */
void lrhtask(void)
{
    cs(ADDRESS_OF(FLGWRD11));
    mask(LRBYBIT);
    if (A_is_zero()) {
        grp2off();
        return;
    }
    set_A(READLBIT);
    mask_at(ADDRESS_OF(FLGWRD11)); /* "IS READLR FLAG SET?" */
    if (A_is_zero()) {             /* "NO.  BYPASS LR READ." */
        grp2off();
        return;
    }
    cs(ADDRESS_OF(FLGWRD11));
    mask(NOLRRBIT);    /* "IS LR READ INHIBITED?" */
    if (A_is_zero()) { /* "YES.  BYPASS LR READ." */
        grp2off();
        return;
    }
    set_A(PRIO32);
    tc_to(LRHTASK + 015u, NOVAC);
}

/* GRP2OFF: restart group 2 off, and on to R10,R11 (R10,R11A). */
void grp2off(void)
{
    group_off(ADDRESS_OF(MINUS_PHASE2));
    continue_at(R10_R11A);
}

#endif
