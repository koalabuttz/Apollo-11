/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 7: NVSUB, the display for the
 * program's own use, and the routines around it -- LM pp. 451-459, CM pp.
 * 369-377. The faithful C is the port's Pinball modules
 * (c64-port/src/lanec_pw_*.c, lanec_pin_head_nvsub.c and the dsky_*.c they
 * run).
 *
 * "NVSUB IS USED FOR SUBROUTINE CALLS FROM WITHIN COMPUTER. IT CAN BE USED
 * TO CALL THE COMBINATION OF ANY DISPLAY, LOAD, OR MONITOR VERB TOGETHER
 * WITH ANY NOUN AVAILABLE TO THE KEYBOARD." The program puts the verb and
 * noun in A (0VVVVVVVNNNNNNN) and calls NVSUB: if the display is in use
 * (DSPLOCK, or an external monitor) it comes back at once to the calling
 * word +1; if not, NVSUB1 shows the verb and noun and runs the verb as if
 * keyed (ENTPAS0), and it comes back to the calling word +2 through ENTRET
 * and NVSUBEND. A negative A blanks the display instead (BLANKDSP).
 * NVMONOPT is NVSUB with options for a monitor verb in L.
 *
 * A program that asks the crew for data waits in ENDIDLE, asleep; RECALTST
 * wakes it when the data is in, or at verb 33 or 34 (PROCEED, TERMINATE),
 * or JAMPROC and JAMTERM, which do those verbs for a program. BLANKSUB
 * blanks any of R1, R2 and R3, and DSPMM puts the major mode on the DSKY
 * by a job (DSPMMJB).
 *
 * Each function runs from a label, or from the word a call returns to, to
 * a call or jump to another routine or to the next label the faithful port
 * enters, which ends it with Q and Z set. A call the faithful port runs
 * within its step is a C call: FLASHOFF at BLANKDSP, ISCADR+0 and ISLIST+0
 * at ENDIDLE, TESTBIT and 5BLANK at BLNKSUB1, 2BLANK at DSPMMJB, NOVAC at
 * DSPMM, JOBWAKE at RECAL1, RELDSP at RECAL3, and SUPDXCHZ, the way back to
 * the caller, at NVSUBEND (but not at BLNKSUB1, whose step ends at SUPDXCHZ
 * +1). */
#include "pinball.h"
#include "../executive/executive.h"
#include "../agc/names.h"

/* ---- NVSUB (LM p. 452, CM p. 370, fixed-fixed) ---- */

static void nvsbcom_plus1(void);

/* NVSUB -- "ZERO NVMONOPT OPTIONS" (LXCH 7), and on as NVMONOPT. */
void nvsub(void)
{
    zl();
    continue_at(NVMONOPT);
}

/* NVMONOPT -- the verb and noun into NVTEMP. With the external monitor
 * bit (BIT14 of MONSAVE1) or DSPLOCK set, "DSP SYST BLOCKED, RET TO 1+
 * CALLING LOC"; with both clear, "DSP SYST AVAILABLE": one more onto the
 * return at NVSBCOM. */
void nvmonopt(void)
{
    if (ts(ADDRESS_OF(NVTEMP))) not_modelled();
    set_A(BIT14);
    mask_at(ADDRESS_OF(MONSAVE1)); /* "EXTERNAL MONITOR BIT" */
    ad_at(ADDRESS_OF(DSPLOCK));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_return(Q_ADDRESS); return;
    case IS_PLUS_ZERO:
        set_A(ONE);
        continue_at(NVSBCOM);
        return;
    case NEGATIVE: continue_at(NVSBCOM); return;
    case IS_MINUS_ZERO: nvsbcom_plus1(); return;
    }
}

/* NVSBCOM -- "2+ CALLING LOC INTO NVQTEM", "STORE NVMONOPT OPTIONS" (L)
 * in MONSAVE2, and "TURN ON KILL MONITOR BIT" (KILMONON, part 5): this
 * display ends any monitor running. */
void nvsbcom(void)
{
    ad_at(Q_ADDRESS);
    nvsbcom_plus1();
}

static void nvsbcom_plus1(void)
{
    if (ts(ADDRESS_OF(NVQTEM))) not_modelled();
    lxch(ADDRESS_OF(MONSAVE2));
    tc(NVSBCOM + 3u, KILMONON);
}

/* NVSUBCOM, where KILMONON returns. Pinball's banks (NVSBBBNK, the BBCON
 * of NVSUB1) into BBANK, the caller's BBANK with its "OLD SUPERBITS" into
 * NVBNKTEM, Pinball's superbits (PINSUPBT) out, and "GO TO NVSUB1 THRU
 * STANDARD LOC" (NVSUBB, in bank 41). */
void nvsubcom(void)
{
    set_A(NVSBBBNK);
    xch(BBANK_ADDRESS);
    ror_superbnk();
    ts(ADDRESS_OF(NVBNKTEM));
    set_A(PINSUPBT);
    write_superbnk();
    tc(NVSUBCOM + 8u, NVSUBB);
}

/* NVSUBEND -- back to the caller: the return and its banks (NVQTEM,
 * NVBNKTEM: "NVBNKTEM MUST = NVQTEM+1") to SUPDXCHZ, "DTCB WITH SUPERBIT
 * SWITCHING". */
void nvsubend(void)
{
    dxch(ADDRESS_OF(NVQTEM));
    tc_at(NVSUBEND + 1u);
    supdxchz();
}

/* ---- NVSUB1 and BLANKDSP (LM pp. 453-454, CM pp. 371-372, bank 41) ---- */

/* BLANKDSP -- "BLANKS DISPLAY ACCORDING TO OPTION NUMBER IN NVTEMP AS
 * FOLLOWS: -4 FULL BLANK, -3 LEAVE MODE, -2 LEAVE MODE AND VERB, -1 BLANK
 * R'S ONLY." A comes from NVSUB1's CCS NVTEMP, 3, 2, 1 or 0, and with
 * SEVEN added is the last of the DSPTAB words to blank (CODE, "7,8,9, OR
 * 10"). Each word from there down to DSPTAB +0 gets the blank relay word,
 * -BIT12 (CS BIT12), and NOUT one more for each word that was +. Then the
 * registers blanked are zeroed (INDEX NVTEMP; TC +5: VERBREG for -4 and
 * -3, NOUNREG for -2 too, CLPASS for all; MODREG never), the keys blocked (DSPCOUNT -VD1), the flash off
 * ("PROTECT AGAINS INVISIBLE FLASH"), and to ENTSET -2, which "ZEROS
 * REQRET". */
void blankdsp(void)
{
    word to;
    bool more = true;

    ad(SEVEN);
    inhint();
    while (more) {
        /* BLANKDSP +2 */
        ts(ADDRESS_OF(CODE)); /* "BLANK SPECIFIED DSPTABS" */
        set_A(negate(BIT12));
        xch(indexed_erasable(ADDRESS_OF(DSPTAB), fetch(ADDRESS_OF(CODE))));
        switch (ccs_at(A_ADDRESS)) {
        case POSITIVE:
            incr(ADDRESS_OF(NOUT));
            tc_at(BLANKDSP + 8u); /* TC +1 */
            break;
        case IS_PLUS_ZERO: tc_at(BLANKDSP + 8u); break; /* TC +1 */
        case NEGATIVE: break;
        case IS_MINUS_ZERO: /* to the TC BLANKDSP +2, with A +0 */ tc_at(BLANKDSP + 10u); continue;
        }
        switch (ccs_at(ADDRESS_OF(CODE))) {
        case POSITIVE: tc_at(BLANKDSP + 10u); break; /* TC BLANKDSP +2: the next one down */
        case IS_PLUS_ZERO: more = false; break;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* CODE counts down from 7 to 10 */
        }
    }
    relint();
    /* INDEX NVTEMP; TC +5: -4 to TC +1 ("NEVER TOUCH MODREG"), -3 to TS
     * VERBREG, -2 to TS NOUNREG, -1 to TS CLPASS. A is +0. */
    to = indexed_by(BLANKDSP + 18u, fetch(ADDRESS_OF(NVTEMP)));
    if (to < BLANKDSP + 14u || to > BLANKDSP + 17u) not_modelled();
    tc_at(BLANKDSP + 13u);
    if (to == BLANKDSP + 14u) tc_at(BLANKDSP + 14u); /* TC +1 */
    if (to <= BLANKDSP + 15u) ts(ADDRESS_OF(VERBREG));
    if (to <= BLANKDSP + 16u) ts(ADDRESS_OF(NOUNREG));
    ts(ADDRESS_OF(CLPASS));
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    tc_at(BLANKDSP + 20u);
    flashoff();
    tc(BLANKDSP + 21u, ENTSET - 2u);
}

/* NVSUB1 -- ENTRET gets ENTSET's word, TC NVSUBEND ("SET RETURN TO
 * NVSUBEND"), for the end of the verb. Then by NVTEMP ("WHAT NOW"): + is
 * the "NORMAL NVSUB CALL (EXECUTE VN OR PASTE)", - blanks (BLANKDSP), and
 * +0 or -0 is a program alarm (GODSPALM). The noun, the low seven bits,
 * into MPAC +3 and the verb, right seven by EDOP, into MPAC +4 ("CANT USE
 * MPAC. DSPDECVN USES MPAC, +1, +2"). "IF NOUN NOT +0, GO ON" (NVSUB2);
 * "IF NOUN = +0, DISPLAY VERB, THEN RETURN" (UPDATVB -1, back at NVSUB1
 * +18). */
void nvsub1(void)
{
    set_A(fetch_fixed(ENTSET));
    ts(ADDRESS_OF(ENTRET));
    switch (ccs_at(ADDRESS_OF(NVTEMP))) {
    case POSITIVE: tc_at(NVSUB1 + 3u); break; /* TC +4 */
    case IS_PLUS_ZERO: tc(NVSUB1 + 4u, GODSPALM); return;
    case NEGATIVE: tc(NVSUB1 + 5u, BLANKDSP); return;
    case IS_MINUS_ZERO: tc(NVSUB1 + 6u, GODSPALM); return;
    }
    set_A(LOW7);
    mask_at(ADDRESS_OF(NVTEMP));
    ts(ADDRESS_OF(MPAC) + 3u);
    ca(ADDRESS_OF(NVTEMP));
    ts(ADDRESS_OF(EDOP)); /* "RIGHT 7" */
    ca(ADDRESS_OF(EDOP));
    ts(ADDRESS_OF(MPAC) + 4u);
    switch (ccs_at(ADDRESS_OF(MPAC) + 3u)) { /* "TEST NOUN" */
    case POSITIVE: tc(NVSUB1 + 15u, NVSUB2); return;
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(MPAC) + 4u);
        tc(NVSUB1 + 17u, UPDATVB - 1u);
        return;
    case NEGATIVE: tc(NVSUB1 + 17u, UPDATVB - 1u); return;
    case IS_MINUS_ZERO: continue_at(NVSUB1 + 18u); return;
    }
}

/* NVSUB1 +18 (ENTSET -2), where UPDATVB returns and BLANKDSP goes: "ZERO
 * REQRET SO THAT PASTED VERBS CAN BE EXECUTED BY OPERATOR." */
void nvsub1_plus18(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(REQRET));
    continue_at(ENTSET);
}

/* ENTSET -- to NVSUBEND, back to the caller. */
void entset(void)
{
    tc(ENTSET, NVSUBEND);
}

/* NVSUB2 -- "TEST VERB". "IF VERB = +0, DISPLAY NOUN, THEN RETURN"
 * (UPDATNN -1, back at NVSUB2 +4). "IF VERB NOT +0, GO ON": MPAC +2,
 * which may hold a machine address for the noun ("TEMP FOR MACH CADR TO
 * BE SPEC. (DSPDECVN USES MPAC, +1, +2)"), into MPAC +5, and the verb on
 * the DSKY (UPDATVB -1, back at NVSUB2 +9). */
void nvsub2(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC) + 4u)) {
    case POSITIVE: tc_at(NVSUB2 + 1u); break; /* TC +4 */
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(MPAC) + 3u);
        tc(NVSUB2 + 3u, UPDATNN - 1u);
        return;
    case NEGATIVE: tc(NVSUB2 + 3u, UPDATNN - 1u); return;
    case IS_MINUS_ZERO: continue_at(NVSUB2 + 4u); return;
    }
    ca(ADDRESS_OF(MPAC) + 2u);
    ts(ADDRESS_OF(MPAC) + 5u);
    ca(ADDRESS_OF(MPAC) + 4u);
    tc(NVSUB2 + 8u, UPDATVB - 1u);
}

/* NVSUB2 +4, where UPDATNN returns: to NVSUBEND. */
void nvsub2_plus4(void)
{
    tc(NVSUB2 + 4u, NVSUBEND);
}

/* NVSUB2 +9, where UPDATVB returns: "IF BOTH NOUN AND VERB NOT +0,
 * DISPLAY BOTH AND GO TO ENTPAS0." The noun on the DSKY (UPDATNN -1). */
void nvsub2_plus9(void)
{
    ca(ADDRESS_OF(MPAC) + 3u);
    tc(NVSUB2 + 10u, UPDATNN - 1u);
}

/* NVSUB2 +11, where UPDATNN returns. LOADSTAT +0, "SET FOR WAITING FOR
 * DATA CONDITION"; CLPASS +0; REQRET +0, "SET REQRET FOR PASS 0."; and
 * MPAC +5 back into MPAC +2, "FOR USE IN INTMCTBS (IN ENTPAS0)". */
void nvsub2_plus11(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(LOADSTAT));
    ts(ADDRESS_OF(CLPASS));
    ts(ADDRESS_OF(REQRET));
    ca(ADDRESS_OF(MPAC) + 5u);
    ts(ADDRESS_OF(MPAC) + 2u);
    continue_at(ENDNVSB1);
}

/* ENDNVSB1 -- the verb runs as if keyed: "IF INTERNAL MACH CADR TO BE
 * SPECIFIED, MPAC+2 WILL BE PLACED INTO NOUNCADR IN ENTPAS0 (INTMCTBS)." */
void endnvsb1(void)
{
    tc(ENDNVSB1, ENTPAS0);
}

/* ---- ENDIDLE (LM p. 455, CM p. 373, fixed-fixed) ---- */

/* DSPABORT -- "TC POODOO; OCT 01206": a second job asleep in Pinball. */
static void dspabort(void)
{
    tc(DSPABORT, POODOO);
}

/* ISLIST+0 -- "ABORTS (CODE 01206) IF DSPLIST NOT= +0. RETURNS IF DSPLIST
 * = +0." True when it went back. A -0 runs the OCT 01206 after DSPABORT,
 * which the readable port does not model. */
static bool islist_returns(void)
{
    switch (ccs_at(ADDRESS_OF(DSPLIST))) {
    case POSITIVE: tc_at(ISLIST_PLUS0 + 1u); break;
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return true;
    case NEGATIVE: break; /* the word after is DSPABORT */
    case IS_MINUS_ZERO: not_modelled();
    }
    dspabort();
    return false;
}

/* ISCADR+0 -- "ABORTS (CODE 01206) IF CADRSTOR NOT= +0. RETURNS IF
 * CADRSTOR = +0." True when it went back. A -0 goes on into ISLIST+0, and
 * back from there. */
static bool iscadr_returns(void)
{
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: tc_at(ISCADR_PLUS0 + 1u); break;
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return true;
    case NEGATIVE: tc_at(ISCADR_PLUS0 + 3u); break;
    case IS_MINUS_ZERO: return islist_returns();
    }
    dspabort();
    return false;
}

/* ENDIDLE -- "ROUTINES THAT REQUEST LOADS THROUGH NVSUB SHOULD USE
 * ENDIDLE WHILE WAITING FOR THE DATA TO BE LOADED. ENDIDLE PUTS CURRENT
 * JOB TO SLEEP." "RETURN ADDRESS INTO L." It aborts with 1206 if a job is
 * asleep in Pinball already: in ENDIDLE (CADRSTOR not +0) or waiting for
 * the display (DSPLIST not +0). The return, as a CADR ("SAME STRATEGY FOR
 * CADR AS MAKECADR": the low ten bits, plus FBANK), into CADRSTOR, which
 * RECALTST wakes; then JOBSLEEP. "DONT SET DSPLOC TO 1 SO CAN USE ENDIDLE
 * WITH NVSUB INITIATED MONITOR." */
void endidle(void)
{
    lxch(Q_ADDRESS);
    tc_at(ENDIDLE + 1u);
    if (!iscadr_returns()) return;
    tc_at(ENDIDLE + 2u);
    if (!islist_returns()) return;
    ca(L_ADDRESS);
    mask(LOW10);
    ad_at(FBANK_ADDRESS);
    if (ts(ADDRESS_OF(CADRSTOR))) not_modelled();
    tc(ENDIDLE + 7u, JOBSLEEP);
}

/* ---- JAMTERM and JAMPROC (LM pp. 455-456, CM pp. 373-374) ---- */

/* JAMTERM -- "ALLOWS PROGRAMS TO PERFORM THE TERMINATE FUNCTION. IT DOES
 * ENDOFJOB." Pinball's superbits, REQRET 34 ("LEAVE ENTER SET FOR
 * ENTPASS0."), the keys blocked (DSPCOUNT -VD1), and verb 34 (VBTERM,
 * part 6) by POSTJUMP. */
void jamterm(void)
{
    set_A(PINSUPBT);
    write_superbnk();
    set_A(THIRTY_FOUR_DEC);
    ts(ADDRESS_OF(REQRET));
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    tc(JAMTERM + 7u, POSTJUMP); /* CADR VBTERM */
}

/* JAMPROC -- "ALLOWS PROGRAMS TO PERFORM THE PROCEED/PROCEED WITHOUT DATA
 * FUNCTION. IT DOES ENDOFJOB." As JAMTERM, with REQRET 33 and verb 33
 * (VBPROC). */
void jamproc(void)
{
    set_A(PINSUPBT);
    write_superbnk();
    set_A(THIRTY_THREE_DEC);
    ts(ADDRESS_OF(REQRET));
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    tc(JAMPROC + 7u, POSTJUMP); /* CADR VBPROC */
}

/* ---- BLANKSUB (LM pp. 456-457, CM pp. 374-375) ---- */

static void blanksub_plus8(void);
static void blanksub_plus11(void);
static void blanksub_plus12(void);

/* BLANKSUB -- "BLANKS ANY COMBINATION OF R1, R2, R3. CALL WITH BLANKING
 * CODE IN A. BIT1=1 BLANKS R1, BIT2=1 BLANKS R2, BIT3=1 BLANKS R3."
 * "STORE BLANKING CODE IN NVTEMP." With the external monitor bit or
 * DSPLOCK set, "DSP SYST BLOCKED. RET TO 1+ CALLING LOC"; with both clear,
 * "SET RETURN FOR 2+ CALLING LOC" (INCR Q). */
void blanksub(void)
{
    mask(SEVEN);
    ts(ADDRESS_OF(NVTEMP));
    set_A(BIT14);
    mask_at(ADDRESS_OF(MONSAVE1)); /* "EXTERNAL MONITOR BIT" */
    ad_at(ADDRESS_OF(DSPLOCK));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_return(Q_ADDRESS); return;
    case IS_PLUS_ZERO:
        incr(Q_ADDRESS);
        blanksub_plus8();
        return;
    case NEGATIVE: blanksub_plus8(); return;
    case IS_MINUS_ZERO: blanksub_plus11(); return;
    }
}

/* BLANKSUB +8. With a code of +0, "NOTHING TO BLANK. RET TO 2+ CALLING
 * LOC". */
static void blanksub_plus8(void)
{
    switch (ccs_at(ADDRESS_OF(NVTEMP))) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE: blanksub_plus11(); return;
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return;
    case IS_MINUS_ZERO: blanksub_plus12(); return;
    }
}

/* BLANKSUB +11: "SET RETURN FOR 2 + CALLING LOC" in L. */
static void blanksub_plus11(void)
{
    lxch(Q_ADDRESS);
    blanksub_plus12();
}

/* BLANKSUB +12. Pinball's banks (BLNKBBNK, the BBCON of BLNKSUB1) into
 * BBANK; the caller's BBANK with its superbits ("SAVE OLD SUPERBITS.") and
 * the return into BUF, BUF +1; Pinball's superbits out; and to BLNKSUB1. */
static void blanksub_plus12(void)
{
    set_A(BLNKBBNK);
    xch(BBANK_ADDRESS);
    ror_superbnk();
    dxch(ADDRESS_OF(BUF));
    set_A(PINSUPBT);
    write_superbnk();
    tc(BLANKSUB + 20u, BLNKSUB1);
}

/* TESTBIT -- "NVTEMP CONTAINS BLANKING CODE." True for "IF CURRENT BIT =
 * 1, RETURN TO L+1"; false for "IF CURRENT BIT = 0, RETURN TO L+3" (INDEX
 * Q; TC 2), which skips the caller's CAF and TC 5BLANK -1. */
static bool testbit(void)
{
    mask_at(ADDRESS_OF(NVTEMP));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_return(Q_ADDRESS); return true;
    case IS_PLUS_ZERO: tc_at(TESTBIT + 4u); return false;
    case NEGATIVE:
    case IS_MINUS_ZERO: break;
    }
    not_modelled(); /* the code is three bits */
    return false;
}

/* BLNKSUB1 -- "SAVE OLD DSPCOUNT FOR LATER RESTORATION" in BUF +2. For
 * BIT1, BIT2 and BIT3 in turn ("TEST BIT1. SEE IF R1 TO BE BLANKED."),
 * the register blanked by 5BLANK from its first digit (R1D1, R2D1, R3D1).
 * Then "RESTORE DSPCOUNT TO STATE IT HAD BEFORE BLANKSUB.", and "CALL L+2
 * DIRECTLY." by BUF's return and banks: SUPDXCHZ +1, "DTCB WITH SUPERBIT
 * SWITCHING". */
void blnksub1(void)
{
    static const word bits[3] = {BIT1, BIT2, BIT3};
    static const word first_digits[3] = {R1D1, R2D1, R3D1};
    unsigned r;

    ca(ADDRESS_OF(DSPCOUNT));
    ts(ADDRESS_OF(BUF) + 2u);
    for (r = 0; r < 3u; ++r) {
        word at = (word)(BLNKSUB1 + 2u + 4u * r); /* CAF BIT1, BIT2, BIT3 */

        set_A(bits[r]);
        tc_at(at + 1u);
        if (!testbit()) continue;
        set_A(first_digits[r]);
        tc_at(at + 3u);
        if (!five_blank_minus1_returns()) return;
    }
    ca(ADDRESS_OF(BUF) + 2u);
    ts(ADDRESS_OF(DSPCOUNT));
    dxch(ADDRESS_OF(BUF));
    tc(BLNKSUB1 + 17u, SUPDXCHZ + 1u);
}

/* ---- DSPMM (LM pp. 457-458, CM pp. 375-376) ---- */

/* DSPMM -- bank 04. "DSPMM DOES NOT DISPLAY MODREG DIRECTLY. IT PUTS IN
 * EXEC REQUEST WITH PRIO 30000 FOR DSPMMJB AND RETURNS TO CALLER." The
 * return into MPAC, a job at CHARIN's priority (CHRPRIO) for the 2CADR
 * after the TC NOVAC, and back by TC MPAC. With no core set for the job,
 * NOVAC's 1202 alarm instead. "DSPMM MUST BE IN BANK 27 OR LOWER, SO IT
 * CAN BE CALLED VIA BANKCALL." */
void dspmm(void)
{
    word job[2];

    xch(Q_ADDRESS);
    ts(ADDRESS_OF(MPAC));
    inhint();
    set_A(CHRPRIO);
    tc_at(DSPMM + 4u);
    job[0] = fetch_fixed(DSPMM + 5u); /* 2CADR DSPMMJB */
    job[1] = fetch_fixed(DSPMM + 6u);
    novac(job);
    if (Z != DSPMM + 7u) return;
    relint();
    tc(ENDSPMM, ADDRESS_OF(MPAC));
}

static void dspmmjb_plus8(void);

/* DSPMMJB -- "PLACE MAJOR MODE CODE INTO MODREG" ("GETS HERE THRU
 * DSPMM"): DSPCOUNT at the mode lights (MD1), its old value into DSPMMTEM
 * ("SAVE DSPCOUNT"). "IF MODREG IS + OR +0, DISPLAY MODREG" (DSPDECVN,
 * back at DSPMMJB +6), "IF MODREG IS -NZ, DO NOTHING", "IF MODREG IS -0,
 * BLANK MM" (2BLANK). */
void dspmmjb(void)
{
    set_A(MD1);
    xch(ADDRESS_OF(DSPCOUNT));
    ts(ADDRESS_OF(DSPMMTEM));
    switch (ccs_at(ADDRESS_OF(MODREG))) {
    case POSITIVE:
        ad(ONE);
        tc(DSPMMJB + 5u, DSPDECVN);
        return;
    case IS_PLUS_ZERO: tc(DSPMMJB + 5u, DSPDECVN); return;
    case NEGATIVE:
        tc_at(DSPMMJB + 6u); /* TC +2 */
        dspmmjb_plus8();
        return;
    case IS_MINUS_ZERO:
        tc_at(DSPMMJB + 7u);
        if (two_blank_returns()) dspmmjb_plus8();
        return;
    }
}

/* DSPMMJB +6, where DSPDECVN returns: TC +2. */
void dspmmjb_plus6(void)
{
    tc_at(DSPMMJB + 6u);
    dspmmjb_plus8();
}

/* DSPMMJB +8: "RESTORE DSPCOUNT", and the job ends. */
static void dspmmjb_plus8(void)
{
    xch(ADDRESS_OF(DSPMMTEM));
    ts(ADDRESS_OF(DSPCOUNT));
    tc(DSPMMJB + 10u, ENDOFJOB);
}

/* ---- RECALTST (LM pp. 458-459, CM pp. 376-377, bank 40) ---- */

static void recal1_plus1(void);

/* RECALTST -- "RECALTST IS ENTERED DIRECTLY AFTER DATA IS LOADED (OR
 * RESEQUENCE VERB IS EXECUTED), TERMINATE VERB IS EXECUTED, OR PROCEED
 * WITHOUT DATA VERB IS EXECUTED. IT WAKES UP JOB THAT DID TC ENDIDLE." With
 * CADRSTOR +0 no job waits: "NORMAL EXIT IF KEYBOARD INITIATED". */
void recaltst(void)
{
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: tc(RECALTST + 1u, RECAL1); return;
    case IS_PLUS_ZERO: tc(RECALTST + 2u, ENDOFJOB); return;
    case NEGATIVE: continue_at(RECAL1); return;
    case IS_MINUS_ZERO: recal1_plus1(); return;
    }
}

/* RECAL1 -- CADRSTOR emptied, and the job asleep at its address woken
 * (JOBWAKE). Then by LOADSTAT: "+ PROCEED WITHOUT DATA" (DOPROC), "-
 * TERMINATE" (DOTERM), "-0 DATA IN OR RESEQUENCE" (two on, RECAL2), and
 * +0, waiting for data still, the "PATHALOGICAL CASE EXIT". */
void recal1(void)
{
    set_A(ZERO);
    recal1_plus1();
}

static void recal1_plus1(void)
{
    xch(ADDRESS_OF(CADRSTOR));
    inhint();
    tc_at(RECAL1 + 3u);
    jobwake();
    if (Z != RECAL1 + 4u) return; /* JOBWAKE went to CCSHOLE */
    switch (ccs_at(ADDRESS_OF(LOADSTAT))) {
    case POSITIVE: tc(RECAL1 + 5u, DOPROC); return;
    case IS_PLUS_ZERO: tc(RECAL1 + 6u, ENDOFJOB); return;
    case NEGATIVE: tc(RECAL1 + 7u, DOTERM); return;
    case IS_MINUS_ZERO:
        set_A(TWO);
        continue_at(RECAL2);
        return;
    }
}

/* RECAL2 -- the woken job's LOC on by A: "RECALTST TESTS LOADSTAT AND
 * WAKES JOB UP TO, L+1 FOR TERMINATE, L+2 FOR PROCEED WITHOUT DATA, L+3
 * FOR DATA IN, OR RESEQUENCE" ("LOC IS + FOR BASIC JOBS"). JOBWAKE left
 * LOCCTR at the job's core set. "SAVE VERB
 * IN MPAC, NOUN IN MPAC+1 AT TIME OF RESPONSE TO ENDIDLE FOR POSSIBLE
 * LATER TESTING BY JOB THAT HAS BEEN WAKED UP." */
void recal2(void)
{
    word locctr = LOCCTR;

    ad_at(indexed_by(ADDRESS_OF(LOC), locctr));
    if (ts(indexed_erasable(ADDRESS_OF(LOC), locctr))) not_modelled();
    ca(ADDRESS_OF(NOUNREG));
    ts(L_ADDRESS);
    ca(ADDRESS_OF(VERBREG));
    dxch(indexed_erasable(ADDRESS_OF(MPAC), locctr));
    relint();
    continue_at(RECAL3);
}

/* RECAL3 -- the display released (RELDSP), and the job ends. */
void recal3(void)
{
    tc_at(RECAL3);
    reldsp();
    tc(RECAL3 + 1u, ENDOFJOB);
}

/* DOTERM -- terminate: LOC on by +0. */
void doterm(void)
{
    set_A(ZERO);
    tc(DOTERM + 1u, RECAL2);
}

/* DOPROC -- proceed without data: LOC on by one. */
void doproc(void)
{
    set_A(ONE);
    tc(DOPROC + 1u, RECAL2);
}
