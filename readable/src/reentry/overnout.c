/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 873-875 (Comanche055, bank 25): the end of
 * every two-second pass (OVERNOUT, NODISKY, SERVNOUT), and the end of the
 * steering below VQUIT (STEEROFF, L357). The faithful C is
 * c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- OVERNOUT, NODISKY, SERVNOUT -- pp. 873-874 ---- */

/* OVERNOUT, after ENDEXIT's EXIT: "ALL ENTRY DISPLAYS ARE DONE HERE.",
 * ENTRYVN by REGODSPR ("NO ABORT IF DISKY IN USE"), unless ENTRYDSP
 * ("ENTRYDSP =92D B13") is clear: "OMIT DISPLAY." ... */
void overnout(void)
{
    set_A(BIT13); /* "ENTRYDSP =92D B13" */
    mask_at(ADDRESS_OF(CM_FLAGS));
    if (A_is_zero()) { /* EXTEND; BZF NODISKY: "OMIT DISPLAY." */
        nodisky();
        return;
    }
    ca(ADDRESS_OF(ENTRYVN));        /* "ALL ENTRY DISPLAYS ARE DONE HERE." */
    tc_to(OVERNOUT + 5u, BANKCALL); /* CADR REGODSPR */
}

/* ... NODISKY: a waiting job of higher priority first (CHANG1), "PROTECT
 * READACCS GRP 5, IF SIDETRACKED."; ... NEWJOB is +0 or a core set; a -0
 * (never) runs STEEROFF's EXIT as an instruction, a MASK, which changes
 * only A, and goes on into STEEROFF, which loads A again. Not modelled: a
 * negative NEWJOB, which runs the CADR SERVEXIT as an instruction. */
void nodisky(void)
{
    inhint();
    switch (ccs_at(ADDRESS_OF(NEWJOB))) { /* "PROTECT READACCS GRP 5, IF SIDETRACKED." */
    case POSITIVE: tc_to(NODISKY + 2u, CHANG1); return;
    case IS_PLUS_ZERO: servnout(); return;
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: steeroff_after_exit(); return;
    }
}

/* ... and SERVNOUT ("COME HERE FROM P67.3"): SERVEXIT, by POSTJUMP, "AND
 * END AVERAGEG JOB VIA  ENDOFJOB." */
void servnout(void)
{
    tc_to(SERVNOUT, POSTJUMP); /* CADR SERVEXIT */
}

/* ---- STEEROFF, L357 -- p. 875 ---- */

/* STEEROFF, below VQUIT: "DISPLAY WHEN V IS LESS THAN VQUIT.": out of the
 * interpreter, ... */
static const word steeroff_words[] = {
    /* STEEROFF */ ORDER(EXIT),
};

const struct interpretive_code STEEROFF_CODE = {025u, STEEROFF, COUNT(steeroff_words),
                                                steeroff_words};

/* ... "PRECAUTIONARY." EBANK, and P67.1, "START UP REMAINDER OF P67", a
 * job at PRIO16 ("2 LESS THAN NTRYPRIO.", NOVAC); ... */
void steeroff_after_exit(void)
{
    set_A(EBENTRY); /* "PRECAUTIONARY." */
    ts(EBANK_ADDRESS);
    set_A(PRIO16);               /* "2 LESS THAN NTRYPRIO." */
    tc_to(STEEROFF + 4u, NOVAC); /* 2CADR P67.1 */
}

/* ... "4.41 RESTART FOR P67.1 DISPLAY JOB." and "SERVICER 5.3 RESTART."
 * (2PHSCHNG, OCT 00414, OCT 10035); ... */
void steeroff_after_novac(void)
{
    tc_to(STEEROFF + 7u, TWO_PHSCHNG);
}

/* ... and "HEREAFTER, DO LAT, LONG.": GOTOADDR P67.2 (P67.2CAD, the GOTO's
 * own address word below). */
void steeroff_after_2phschng(void)
{
    set_A(P67_2CAD); /* "HEREAFTER, DO LAT, LONG." */
    ts(ADDRESS_OF(GOTOADDR));
    tc_at(STEEROFF + 014u);
    intpret();
}

/* In the interpreter, P67.2: "CONTINUE FOR LAT, LONG THIS TIME." L357,
 * from L350: "L/D = L/DCMINR SIGN(L/D)", into L355. */
static const word steeroff_interpretive_words[] = {
    /* STEEROFF +015 */ ORDER(GOTO),
    /* P67.2CAD */ ADDRESS(P67_2),  /* "CONTINUE FOR LAT, LONG THIS TIME." */
    /* L357 */ ORDERS(DLOAD, SIGN), /* "L/D = L/DCMINR SIGN(L/D)" */
    ADDRESS_PLUS_ONE(L_DCMINR),
    ADDRESS_PLUS_ONE(L_D),
    STCALL(L_D1),
    ADDRESS(L355), /* "(GO TO)" */
};

const struct interpretive_code STEEROFF_INTERPRETIVE_CODE = {
    025u, STEEROFF + 015u, COUNT(steeroff_interpretive_words), steeroff_interpretive_words};

#endif
