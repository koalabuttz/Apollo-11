/* SPDX-License-Identifier: MIT */
/* AGC_BLOCK_TWO_SELF-CHECK.agc, both missions (LM pp. 1284-1293, CM pp.
 * 1394-1403): self-check and the bank sums of SHOW-BANKSUM. The two
 * listings are the same code, Luminary099's two words lower in bank 43.
 * The faithful C is c64-port/src/self_check_body.inc (with lm_ and
 * cm_self_check.c and their _idle files).
 *
 * "PROGRAM HAS TWO MAIN PARTS. THE FIRST IS SELF-CHECK WHICH RUNS AS A
 * ZERO PRIORITY JOB WITH NO CORE SET, AS PART OF THE BACK-UP IDLE LOOP.
 * THE SECOND IS SHOW-BANKSUM WHICH RUNS AS A REGULAR EXECUTIVE JOB WITH
 * ITS OWN STARTING VERB." SMODE picks what self-check does (NOUN 27):
 * "+-4 ERASABLE MEMORY", "+-5 FIXED MEMORY", "+-1,2,3,6,7,10 EVERYTHING
 * IN OPTIONS 4 AND 5." (this version has no other checks: those options
 * go straight on, through SKEEP1), "+0 NO CHECK, PUTS COMPUTER INTO THE
 * BACKUP IDLE LOOP." On an error, "+ OPTION NUMBERS PUT COMPUTER INTO
 * BACKUP IDLE LOOP, - OPTION NUMBERS RESTART THE OPTION.", and "THE -0
 * OPTION PROCEEDS FROM THE LINE FOLLOWING THE LINE WHERE THE ERROR WAS
 * DETECTED."
 *
 * Self-check runs whenever the Executive has nothing else: ADVAN, with
 * NEWJOB -0, goes on at the address in SELFRET, and between steps
 * self-check gives the Executive back its chance (CHECKNJ: POSTJUMP
 * ADVAN). So every pass leaves the file, and the places it comes back to
 * are routines of their own here, named by what they follow
 * (eloopfin_plus2(), back from CHECKNJ). So are the places the -0 option
 * resumes at after an error, through SFAIL, and those SKEEP1 sends the
 * loop on to.
 *
 * Each label's routine is its lower-case name; a TC to another label is
 * a call to it, after tc_at() sets Q as the TC does. -1CHK and ADSUM,
 * which end with TC Q, return to their caller.
 *
 * The binding (src/binding/self_check_routines.c) runs these in bank 43
 * with the rope's words for the fixed memory FXADRS reads. */
#include "self_check.h"
#include "../agc/names.h"

static void prerrors(void);
static void sidloop(void);
static void soptions(void);
static void erasloop(void);
static void eloopfin(void);
static void e134567b(void);
static void two_ebank(void);
static void noebank(void);
static void cntrchk(void);
static void commfx(void);
static void comadrs(void);
static void fxfx(void);
static void fxadrs(void);
static void adrschk(void);
static void continu(void);
static void continu_plus1(void);
static void gonxtbnk(void);
static void soption(void);

/* ---- Indexing, as the faithful port takes it ---- */

/* The address INDEX gives with a word (the overflow-corrected word; -0
 * adds nothing, as +0). */
static word index_value(reg16 value)
{
    word index = overflow_corrected(value);
    return index == MINUS_ZERO ? PLUS_ZERO : index;
}

/* NDX k before DCA, DCS or DXCH 0000: the pair of erasable words X and
 * X +1 at the address k holds. The faithful port takes a pair at or above
 * 061 once the EB window is placed (ERASCHK's own range); any other stops
 * the run there. */
static unsigned pair_at(word address)
{
    if (address >= 01777u || erasable_index(address) < 061u ||
        (address == 01377u && erasable_index(01400u) < 061u))
        not_modelled();
    return address;
}

/* NDX SKEEP7 before CS or AD 0000 or 0001: X, the lower of the pair. */
static unsigned x_at(void)
{
    if (SKEEP7 >= 01777u) not_modelled();
    return SKEEP7;
}

/* TC to an erasable register, TC Q, TC SFAIL or TC SKEEP1: the AGC runs
 * the register's word as an instruction, a TC to the address it holds.
 * Q gets the register's address +1. The faithful port takes it only as a
 * TC to fixed memory. */
static void tc_register(unsigned address)
{
    word to = (word)(agc.memory[erasable_index(address)] & MINUS_ZERO);

    if (to < 02000u || to > 07777u) not_modelled();
    tc_through(address);
    continue_at(to);
}

/* ---- Errors ---- */

/* -1CHK -- LM p. 1287, CM p. 1397.
 * A must be -1: back to the caller (TC Q) if it is, else PRERRORS. CCS A
 * twice: -1 is the one value whose first CCS is negative and whose
 * second, on |A| - 1, is +0 (|A| - 1 is never negative). */
static bool minus_1chk(void)
{
    if (ccs_register(A_ADDRESS) == NEGATIVE && ccs_register(A_ADDRESS) == IS_PLUS_ZERO) {
        tc_register(Q_ADDRESS); /* TC Q */
        return true;
    }
    prerrors(); /* TCF PRERRORS */
    return false;
}

/* PRERRORS, ERRORS -- LM p. 1286, CM p. 1396.
 * "IS IT NECESSARY TO RESTORE ERASABLE": an error in ERASLOOP, with the
 * pair under test holding its own addresses, puts back the words saved
 * in SKEEP5 and SKEEP6 ("RESTORE THE TWO ERASABLE REGISTERS"). Then ERRORS
 * saves Q, the address after the failing TC -1CHK, in SFAIL ("SAVE Q FOR
 * FAILURE LOCATION") and ALMCADR ("FOR DISPLAY WITH BBANK AND ERCOUNT"),
 * counts the error and goes to ALARM2 with "SELF-CHECK MALFUNCTION
 * INDICATOR" 01102. A TS of Q that overflows is not modelled: Q holds
 * the return address. */
static void prerrors(void)
{
    ca(ADDRESS_OF(ERESTORE));
    if (!A_is_zero()) { /* EXTEND; BZF ERRORS: "NO" */
        dca(ADDRESS_OF(SKEEP5));
        dxch(pair_at(SKEEP7)); /* INDEX SKEEP7; DXCH 0000 */
        set_A(S_PLUS_ZERO);
        ts(ADDRESS_OF(ERESTORE));
    }
    inhint(); /* ERRORS */
    ca(Q_ADDRESS);
    if (ts(ADDRESS_OF(SFAIL))) not_modelled();
    ts(ADDRESS_OF(ALMCADR));
    incr(ADDRESS_OF(ERCOUNT)); /* "KEEP TRACK OF NUMBER OF MALFUNCTIONS." */
    tc_at(TCALARM2);           /* TC ALARM2; OCT 01102 */
    continue_at(ALARM2);
}

/* TCALARM2 +2 -- LM p. 1286, CM p. 1396.
 * Back from ALARM2: a positive option or +0 goes to the idle loop
 * (SIDLOOP: +0 into SMODE, TC SELFCHK; for +0, A is +0 already), a
 * negative one to the idle loop with SMODE kept ("GO TO IDLE LOOP"), so
 * SMODECHK starts it again; -0 goes on after the failing check ("CONTINUE
 * WITH SELF-CHECK"). */
void tcalarm2_plus2(void)
{
    switch (ccs_at(ADDRESS_OF(SMODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: sidloop(); return;
    case NEGATIVE:
        tc_at(SIDLOOP + 2u); /* TC SELFCHK */
        continue_at(SELFCHK);
        return;
    case IS_MINUS_ZERO: tc_register(ADDRESS_OF(SFAIL)); return; /* TC SFAIL */
    }
}

/* SIDLOOP -- LM p. 1286, CM p. 1396.
 * Self-check off (SMODE +0), and back to the idle loop. */
static void sidloop(void)
{
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(SMODE));
    tc_at(SIDLOOP + 2u); /* TC SELFCHK: "GO TO IDLE LOOP" */
    continue_at(SELFCHK);
}

/* ---- The idle loop ---- */

/* SELFCHK -- LM p. 1287, CM p. 1397.
 * "CHARLEY, COME IN HERE": where the Executive's idle job starts
 * (SELFADRS). */
void selfchk(void)
{
    tc_at(SELFCHK); /* TC SMODECHK */
    smodechk();
}

/* SMODECHK -- LM p. 1287, CM p. 1397.
 * Keep the caller's return in SKEEP1, and let the Executive run any job
 * that has come up ("CHECK FOR NEW JOB"). */
void smodechk(void)
{
    qxch(ADDRESS_OF(SKEEP1)); /* EXTEND; QXCH SKEEP1 */
    smodechk_plus2();
}

void smodechk_plus2(void)
{
    tc_at(SMODECHK + 2u); /* TC CHECKNJ */
    checknj();
}

/* SMODECHK +3 -- LM p. 1287, CM p. 1397.
 * Back from CHECKNJ with no job waiting: CCS SMODE. +0 goes round the
 * idle loop again ("TO BACKUP IDLE LOOP"). The other outcomes go on at
 * the word the CCS picks, each a routine of its own: the flight program
 * loads the idle loop alone (c64-port/docs/FLIGHT-STORE.md), and its run
 * ends where self-check proper starts. */
void smodechk_plus3(void)
{
    switch (ccs_at(ADDRESS_OF(SMODE))) {
    case POSITIVE: continue_at(SMODECHK + 4u); return;      /* TC SOPTIONS */
    case IS_PLUS_ZERO: smodechk_plus5(); return;            /* TC SMODECHK +2 */
    case NEGATIVE: continue_at(SMODECHK + 6u); return;      /* TC SOPTIONS */
    case IS_MINUS_ZERO: continue_at(SMODECHK + 7u); return; /* INCR SCOUNT */
    }
}

void smodechk_plus5(void)
{
    tc_at(SMODECHK + 5u); /* TC SMODECHK +2 */
    smodechk_plus2();
}

/* CHECKNJ -- LM p. 1287, CM p. 1397.
 * "SAVE RETURN ADDRESS WHILE TESTING NEWJOB" in SELFRET, and go to ADVAN
 * by POSTJUMP ("TO SEE IF ANY JOBS HAVE BECOME ACTIVE."): with none, it
 * comes back at SELFRET's address. */
void checknj(void)
{
    qxch(ADDRESS_OF(SELFRET)); /* EXTEND; QXCH SELFRET */
    checknj_plus2();
}

void checknj_plus2(void)
{
    tc_at(CHECKNJ + 2u); /* TC POSTJUMP; CADR ADVAN */
    continue_at(POSTJUMP);
}

/* ---- The options ---- */

/* SMODECHK +4, +6 -- LM p. 1287, CM p. 1397.
 * A positive or negative option: SOPTIONS, with |SMODE| - 1 in A. */
void smodechk_plus4(void)
{
    tc_at(SMODECHK + 4u); /* TC SOPTIONS */
    soptions();
}

void smodechk_plus6(void)
{
    tc_at(SMODECHK + 6u); /* TC SOPTIONS */
    soptions();
}

/* SMODECHK +7, +010 -- LM p. 1287, CM p. 1397.
 * The -0 option: count the pass and go on where SKEEP1 says ("CONTINUE
 * WITH SELF-CHECK"). */
void smodechk_plus7(void)
{
    incr(ADDRESS_OF(SCOUNT));
    smodechk_plus8();
}

void smodechk_plus8(void)
{
    tc_register(ADDRESS_OF(SKEEP1)); /* TC SKEEP1 */
}

/* SOPTIONS, BNKOPTN, SOPTION1-SOPTON10 -- LM p. 1287, CM p. 1397.
 * Options 1 to 8 (|SMODE| - 1 is 0 to 7, "FOR OPTIONS BELOW NINE."): count
 * the pass and take the option's TC, INDEX A; TC SOPTION1. Option 4 is
 * ERASCHK, 5 ROPECHK, and the rest TC SKEEP1 ("WAS TC+TCF", "WAS
 * IN:OUT1", "WAS COUNTCHK": checks this version left out). A larger one
 * is an "ILLEGAL OPTION. GO TO IDLE LOOP." */
static void soptions(void)
{
    word option;

    ad(S_MINUS_7);
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        tc_at(BNKOPTN);          /* TC SIDLOOP */
        sidloop();
        return;
    }
    incr(ADDRESS_OF(SCOUNT));
    ad(S_PLUS_7);
    option = index_value(A);
    if (option > 7u) not_modelled(); /* (never: the BZMF keeps A at 0 to 7) */
    tc_at(BNKOPTN + 4u);             /* INDEX A; TC SOPTION1 */
    switch (option) {
    case 3u:
        tc_at(SOPTION4); /* TC ERASCHK */
        eraschk();
        return;
    case 4u:
        tc_at(SOPTION5); /* TC ROPECHK */
        ropechk();
        return;
    default:
        tc_at((word)(SOPTION1 + option)); /* TC SKEEP1 */
        tc_register(ADDRESS_OF(SKEEP1));
        return;
    }
}

/* ---- ERASCHK: erasable memory ---- */

/* ERASCHK, 0EBANK -- LM p. 1288, CM p. 1398.
 * "ERASCHK TAKES APPROXMATELY 7 SECONDS": every pair of erasable words,
 * one pair a pass. "SKEEP7 HOLDS LOWEST OF TWO ADDRESSES BEING CHECKED.",
 * "SKEEP3 HOLDS LAST ADDRESS BEING CHECKED (HIGHEST ADDRESS).", and
 * "SKEEP2 CONTROLS CHECKING OF NON-SWITCHABLE ERASABLE MEMORY WITH BANK
 * NUMBERS IN EB." First E bank 0 from 01461 (physical 061, above the
 * registers) to 01777. */
void eraschk(void)
{
    set_A(S_PLUS_1);
    ts(ADDRESS_OF(SKEEP2));
    set_A(S_PLUS_ZERO); /* 0EBANK */
    ts(EBANK_ADDRESS);
    set_A(ERASCON3);
    ts(ADDRESS_OF(SKEEP7)); /* "STARTING ADDRESS" */
    set_A(S10BITS);
    ts(ADDRESS_OF(SKEEP3)); /* "LAST ADDRESS CHECKED" */
    tc_at(ZERO_EBANK + 6u); /* TC ERASLOOP */
    erasloop();
}

/* E134567B -- LM p. 1288, CM p. 1398.
 * E banks 1 and 3-7: 01400 to 01777. */
static void e134567b(void)
{
    set_A(ERASCON6);
    ts(ADDRESS_OF(SKEEP7));
    set_A(S10BITS);
    ts(ADDRESS_OF(SKEEP3));
    tc_at(E134567B + 4u); /* TC ERASLOOP */
    erasloop();
}

/* 2EBANK -- LM p. 1288, CM p. 1398.
 * E bank 2: 01400 to 01773. */
static void two_ebank(void)
{
    set_A(ERASCON6);
    ts(ADDRESS_OF(SKEEP7));
    set_A(ERASCON4);
    ts(ADDRESS_OF(SKEEP3));
    tc_at(TWO_EBANK + 4u); /* TC ERASLOOP */
    erasloop();
}

/* NOEBANK -- LM p. 1288, CM p. 1398.
 * The unswitched words, 00061 to 01373 (below SKEEP5-7, which hold the
 * pair), with SKEEP2 +0 (A, from CCS SKEEP2 +1). After each E bank: so
 * they are checked with each bank in EB. */
static void noebank(void)
{
    ts(ADDRESS_OF(SKEEP2)); /* "+0" */
    set_A(ERASCON1);
    ts(ADDRESS_OF(SKEEP7));
    set_A(ERASCON2);
    ts(ADDRESS_OF(SKEEP3));
    erasloop();
}

/* ERASLOOP -- LM pp. 1288-1289, CM pp. 1398-1399.
 * One pair X, X +1, with interrupts off: save the words (B(X), B(X+1))
 * in SKEEP5 and SKEEP6 and X's address in ERESTORE ("IF RESTART, RESTORE
 * C(X) AND C(X+1)"), store each word's own address in it ("PUTS OWN
 * ADDRESS IN X AND X +1") and check that X - (X +1) is -1. */
static void erasloop(void)
{
    inhint();
    ca(EBANK_ADDRESS);
    ts(ADDRESS_OF(SKEEP4));   /* "STORES C(EBANK)" */
    dca(pair_at(SKEEP7));     /* EXTEND; NDX SKEEP7; DCA 0000 */
    dxch(ADDRESS_OF(SKEEP5)); /* "STORES C(X) AND C(X+1) IN SKEEP6 AND 5." */
    ca(ADDRESS_OF(SKEEP7));
    ts(ADDRESS_OF(ERESTORE));
    ts(L_ADDRESS);
    incr(L_ADDRESS);
    dxch(pair_at(index_value(A))); /* NDX A; DXCH 0000 */
    cs(x_at() + 1u);               /* NDX SKEEP7; CS 0001: "CS X+1" */
    ad_at(x_at());                 /* NDX SKEEP7; AD 0000: "AD X" */
    tc_at(ERASLOOP + 021u);        /* TC -1CHK */
    if (minus_1chk()) erasloop_plus022();
}

/* ERASLOOP +022 -- LM pp. 1288-1289, CM pp. 1398-1399.
 * Unless a restart put the words back already ("HAS ERASABLE BEEN
 * RESTORED", ERESTORE +0), store the complements of the addresses
 * ("PUT COMPLEMENT OF ADDRESS OF X AND X+1") and check (X +1) - X is
 * -1. */
void erasloop_plus022(void)
{
    ca(ADDRESS_OF(ERESTORE));
    if (A_is_zero()) { /* EXTEND; BZF ELOOPFIN: "YES, EXIT ERASLOOP." */
        eloopfin();
        return;
    }
    dcs(pair_at(SKEEP7));   /* EXTEND; NDX SKEEP7; DCS 0000 */
    dxch(pair_at(SKEEP7));  /* NDX SKEEP7; DXCH 0000 */
    cs(x_at());             /* NDX SKEEP7; CS 0000: "CS X" */
    ad_at(x_at() + 1u);     /* NDX SKEEP7; AD 0001: "AD X+1" */
    tc_at(ERASLOOP + 036u); /* TC -1CHK */
    if (minus_1chk()) erasloop_plus037();
}

/* ERASLOOP +037 -- LM p. 1289, CM p. 1399.
 * "PUT B(X) AND B(X+1) BACK INTO X AND X+1", and clear ERESTORE ("IF
 * RESTART, DO NOT RESTORE C(X), C(X+1)"). */
void erasloop_plus037(void)
{
    ca(ADDRESS_OF(ERESTORE));
    if (A_is_zero()) { /* EXTEND; BZF ELOOPFIN */
        eloopfin();
        return;
    }
    dca(ADDRESS_OF(SKEEP5));
    dxch(pair_at(SKEEP7)); /* NDX SKEEP7; DXCH 0000 */
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(ERESTORE));
    eloopfin();
}

/* ELOOPFIN -- LM p. 1289, CM p. 1399.
 * Interrupts back on, and the Executive's turn ("CHECK FOR NEW JOB"). */
static void eloopfin(void)
{
    relint();
    tc_at(ELOOPFIN + 1u); /* TC CHECKNJ */
    checknj();
}

/* ELOOPFIN +2 -- LM p. 1289, CM p. 1399.
 * Back from CHECKNJ: EBANK as it was ("REPLACES B(EBANK)") and the next
 * pair ("GO TO NEXT ADDRESS IN SAME BANK"). At a bank's end, CCS SKEEP2:
 * +1 after an E bank, the unswitched words (NOEBANK); +0 after those, the
 * next E bank ("PUT +1 IN SKEEP2."; E bank 2 to 01773, "CHECK FOR BANK
 * E2"). After E bank 7 the TS leaves EBANK 0 (it keeps only the bank
 * bits), and the CCS goes on at CA ERASCON6: "END OF ERASCHK", EBANK 3,
 * and CNTRCHK. (SKEEP2 is never negative or -0 here, nor is EBANK.) */
void eloopfin_plus2(void)
{
    ca(ADDRESS_OF(SKEEP4));
    ts(EBANK_ADDRESS);
    incr(ADDRESS_OF(SKEEP7));
    cs(ADDRESS_OF(SKEEP7));
    ad_at(ADDRESS_OF(SKEEP3));
    if (!A_is_zero()) {         /* EXTEND; BZF +2 */
        tc_at(ELOOPFIN + 011u); /* TC ERASLOOP */
        erasloop();
        return;
    }
    switch (ccs_at(ADDRESS_OF(SKEEP2))) {
    case POSITIVE:
        tc_at(ELOOPFIN + 013u); /* TC NOEBANK */
        noebank();
        return;
    case IS_PLUS_ZERO: incr(ADDRESS_OF(SKEEP2)); /* fall through */
    case NEGATIVE: ca(EBANK_ADDRESS);            /* fall through */
    case IS_MINUS_ZERO: break;
    }
    ad(SBIT9);
    ts(EBANK_ADDRESS);
    ad(ERASCON5);
    if (A_is_zero()) { /* EXTEND; BZF 2EBANK */
        two_ebank();
        return;
    }
    switch (ccs_at(EBANK_ADDRESS)) {
    case POSITIVE:
        tc_at(ELOOPFIN + 024u); /* TC E134567B: "GO TO EBANKS 1,3,4,5,6, AND 7" */
        e134567b();
        return;
    case IS_PLUS_ZERO: set_A(ERASCON6); /* fall through */
    case NEGATIVE: ts(EBANK_ADDRESS);   /* fall through */
    case IS_MINUS_ZERO: break;
    }
    cntrchk();
}

/* ---- CNTRCHK and CYCLSHFT: the counters and the editing registers ---- */

/* CNTRCHK, CNTRLOOP -- LM pp. 1289-1290, CM pp. 1399-1400.
 * "CNTRCHK PERFORMS A CS OF ALL REGISTERS FROM OCT. 60 THROUGH OCT. 10.
 * INCLUDED ARE ALL COUNTERS, T6-1, CYCLE AND SHIFT, AND ALL RUPT
 * REGISTERS". SKEEP2 counts down from 050 by CCS, so it ends at +0, and
 * CYCLSHFT starts at its first word. */
static void cntrchk(void)
{
    enum sign count;
    word counter;

    set_A(CNTRCON);
    for (;;) {
        ts(ADDRESS_OF(SKEEP2)); /* CNTRLOOP */
        ad(SBIT4);              /* "+10 OCTAL" */
        counter = index_value(A);
        if (counter < 3u || counter > 01777u) not_modelled(); /* (never: 010 to 060) */
        cs(counter);                                          /* INDEX A; CS 0000 */
        count = ccs_at(ADDRESS_OF(SKEEP2));
        if (count != POSITIVE) break;
        tc_at(CNTRLOOP + 5u); /* TC CNTRLOOP */
    }
    /* CYCLSHFT -- LM p. 1290, CM p. 1400.
     * "CYCLSHFT CHECKS THE CYCLE AND SHIFT REGISTERS": 25252 into each,
     * then the sum of what each holds, read twice (each read edits it
     * again), must come to -1 each time. A negative or -0 SKEEP2 would
     * go on at TS CYR or TS CYL; it never happens. */
    switch (count) {
    case POSITIVE: break;                   /* (not here: the loop went on, TC CNTRLOOP) */
    case IS_PLUS_ZERO: set_A(CONC_PLUS_S1); /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(CYR));     /* fall through: "C(CYR) = 12525" */
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(CYL));  /* "C(CYL) = 52524" */
    ts(ADDRESS_OF(SR));   /* "C(SR) = 12525" */
    ts(ADDRESS_OF(EDOP)); /* "C(EDOP) = 00125" */
    ad_at(ADDRESS_OF(CYR));
    ad_at(ADDRESS_OF(CYL));
    ad_at(ADDRESS_OF(SR));
    ad_at(ADDRESS_OF(EDOP));
    ad(CONC_PLUS_S2);
    tc_at(CYCLSHFT + 012u); /* TC -1CHK */
    if (minus_1chk()) cyclshft_plus013();
}

/* CYCLSHFT +013 -- LM p. 1290, CM p. 1400. */
void cyclshft_plus013(void)
{
    ad_at(ADDRESS_OF(CYR)); /* "45252" */
    ad_at(ADDRESS_OF(CYL)); /* "72523" */
    ad_at(ADDRESS_OF(SR));
    ad_at(ADDRESS_OF(EDOP));
    ad(S_PLUS_1);
    tc_at(CYCLSHFT + 020u); /* TC -1CHK */
    if (minus_1chk()) cyclshft_plus021();
}

/* CYCLSHFT +021 -- LM p. 1290, CM p. 1400.
 * Count the pass of ERASCHK, CNTRCHK and CYCLSHFT in SCOUNT +1, and back
 * to SMODECHK, which keeps ROPECHK, the next word, in SKEEP1. */
void cyclshft_plus021(void)
{
    incr(ADDRESS_OF(SCOUNT) + 1u);
    tc_at(CYCLSHFT + 022u); /* TC SMODECHK */
    smodechk();
}

/* ---- ROPECHK and SHOW-BANKSUM: fixed memory ---- */

/* ROPECHK -- LM p. 1290, CM p. 1400.
 * Sum every bank of fixed memory, one word a pass, and check each sum
 * ("-0 FOR ROPECHK." in SKEEP6). */
void ropechk(void)
{
    set_A(S_MINUS_ZERO);
    ts(ADDRESS_OF(SKEEP6));
    stshosum();
}

/* STSHOSUM -- LM p. 1290, CM p. 1400.
 * Where SHOW-BANKSUM starts too, with SKEEP6 +1: from bank 0 ("BANK
 * NUMBER" in SKEEP4, its superbank bits in the low ones). "SKEEP1 HOLDS
 * SUM", "SKEEP3 HOLDS PRESENT ADDRESS", "SKEEP5 COUNTS 2 SUCCESSIVE TC
 * SELF WORDS" and "SKEEP7 CONTROLS WHEN ROUNTINE IS IN COMMON FIXED OR
 * FIXED FIXED BANKS". */
void stshosum(void)
{
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(SKEEP4));
    set_A(S_PLUS_1);
    commfx();
}

/* COMMFX -- LM p. 1290, CM p. 1400.
 * A bank read through FBANK (common fixed): SKEEP7 positive. */
static void commfx(void)
{
    ts(ADDRESS_OF(SKEEP7));
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(SKEEP1));
    ts(ADDRESS_OF(SKEEP3));
    set_A(S_PLUS_1);
    ts(ADDRESS_OF(SKEEP5)); /* "COUNTS DOWN 2 TC SELF WORDS" */
    comadrs();
}

/* COMADRS -- LM pp. 1290-1291, CM pp. 1400-1401.
 * The word at the bank's address SKEEP3, by SUPDACAL ("SUPER DATA CALL"),
 * with the bank's superbank bits in L ("TO SET SUPER BANK"). */
static void comadrs(void)
{
    ca(ADDRESS_OF(SKEEP4));
    ts(L_ADDRESS);
    mask(HI5);
    ad_at(ADDRESS_OF(SKEEP3));
    tc_at(COMADRS + 4u); /* TC SUPDACAL */
    continue_at(SUPDACAL);
}

/* COMADRS +5, +6 -- LM p. 1291, CM p. 1401.
 * Back from SUPDACAL with the word: add it in (ADSUM leaves SKEEP3 less
 * the word), and ADRSCHK with SKEEP3 as the address in the bank, 02000
 * on. */
void comadrs_plus5(void)
{
    tc_at(COMADRS + 5u); /* TC ADSUM */
    adsum();
    comadrs_plus6();
}

void comadrs_plus6(void)
{
    ad(SBIT11);          /* "02000" */
    tc_at(COMADRS + 7u); /* TC ADRSCHK */
    adrschk();
}

/* FXFX -- LM p. 1291, CM p. 1401.
 * A fixed-fixed bank, read at its own addresses: "04000, STARTING
 * ADDRESS OF BANK 02" (SKEEP7 -1) or "06000, STARTING ADDRESS OF BANK
 * 03" (SKEEP7 -0). */
static void fxfx(void)
{
    com(); /* CS A */
    ts(ADDRESS_OF(SKEEP7));
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        set_A(SBNK03);
    } else {
        set_A(SBIT12);
        tc_at(FXFX + 5u); /* TC +2 */
    }
    ts(ADDRESS_OF(SKEEP3));
    set_A(S_PLUS_ZERO);
    ts(ADDRESS_OF(SKEEP1));
    set_A(S_PLUS_1);
    ts(ADDRESS_OF(SKEEP5)); /* "COUNTS DOWN 2 TC SELF WORDS" */
    fxadrs();
}

/* FXADRS -- LM p. 1291, CM p. 1401.
 * INDEX SKEEP3; CA 0000: the word at the address, read directly. The
 * faithful port reads it below 010000; an erasable address that is Z
 * reads as the address of the next instruction. */
static void fxadrs(void)
{
    word address = index_value(SKEEP3);

    if (address > 07777u) not_modelled();
    if (address < 02000u && erasable_index(address) == Z_ADDRESS)
        set_A(FXADRS + 2u);
    else
        ca(address);
    tc_at(FXADRS + 2u); /* TC ADSUM */
    adsum();
    fxadrs_plus3();
}

void fxadrs_plus3(void)
{
    tc_at(FXADRS + 3u); /* TC ADRSCHK */
    adrschk();
}

/* ADSUM -- LM p. 1291, CM p. 1401.
 * Add the word ("SKEEP2 HOLDS PRESENT CONTENTS OF ADDRESS") into the sum
 * SKEEP1 with an end-around carry, and leave the address minus the word
 * in A. TS skips the next instruction on an overflow: the carry, +-1,
 * then goes into the sum by the next AD; the last TS's overflow leaves
 * A +-1 for the AD SKEEP3. */
void adsum(void)
{
    if (!ts(ADDRESS_OF(SKEEP2))) ad_at(ADDRESS_OF(SKEEP1));
    if (!ts(ADDRESS_OF(SKEEP1))) set_A(S_PLUS_ZERO); /* CAF S+ZERO */
    ad_at(ADDRESS_OF(SKEEP1));
    if (!ts(ADDRESS_OF(SKEEP1))) cs(ADDRESS_OF(SKEEP2));
    ad_at(ADDRESS_OF(SKEEP3));
    tc_register(Q_ADDRESS); /* TC Q */
}

/* ADRSCHK -- LM pp. 1291-1292, CM pp. 1401-1402.
 * After each word, the bank is done at its last address ("CHECKSUM
 * FINISHED IF LAST ADDRESS.") or at the word after two words in a row
 * that are a TC to their own address (the address less the word is +-0:
 * "-0 MEANS A TC SELF WORD."). That word, the bugger word, makes the sum
 * come out ("AD IN THE BUGGER WORD"). SKEEP5 counts: +1, +0 after one
 * such word, -1 after two; any other word sets it back to +1 (CONTINU). */
static void adrschk(void)
{
    lxch(A_ADDRESS);
    ca(ADDRESS_OF(SKEEP3));
    mask(LOW10);       /* "RELATIVE ADDRESS" */
    ad(MINUS_MAXADRS); /* "SUBTRACT MAX RELATIVE ADDRESS = 1777." */
    if (A_is_zero()) { /* EXTEND; BZF SOPTION */
        soption();
        return;
    }
    switch (ccs_at(ADDRESS_OF(SKEEP5))) {            /* "IS CHECKSUM FINISHED" */
    case POSITIVE: tc_at(ADRSCHK + 7u); break;       /* TC +3: "NO" */
    case IS_PLUS_ZERO: tc_at(ADRSCHK + 010u); break; /* TC +2: "NO" */
    case NEGATIVE:
        tc_at(ADRSCHK + 011u); /* TC SOPTION: "GO TO ROPECHK SHOWSUM OPTION" */
        soption();
        return;
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_register(L_ADDRESS)) {
    case POSITIVE:
        tc_at(ADRSCHK + 013u);
        continu();
        return; /* TC CONTINU */
    case IS_PLUS_ZERO:
        tc_at(ADRSCHK + 014u);
        continu();
        return; /* TC CONTINU */
    case NEGATIVE:
        tc_at(ADRSCHK + 015u);
        continu();
        return; /* TC CONTINU */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(ADDRESS_OF(SKEEP5))) {
    case POSITIVE:
        tc_at(ADRSCHK + 017u); /* TC CONTINU +1 */
        continu_plus1();
        return;
    case IS_PLUS_ZERO: set_A(S_MINUS_1); /* fall through */
    case NEGATIVE:
        tc_at(ADRSCHK + 021u); /* TC CONTINU +1 */
        continu_plus1();
        return;
    case IS_MINUS_ZERO: continu(); return;
    }
}

/* CONTINU, CONTINU +1 -- LM p. 1292, CM p. 1402.
 * "MAKE SURE TWO CONSECUTIVE TC SELF WORDS": SKEEP5 +1 (or the count
 * ADRSCHK leaves). Then the next word: ROPECHK lets the Executive in
 * first ("-0 IN SKEEP6 FOR ROPECHK"); SHOW-BANKSUM (SKEEP6 +1) goes to
 * CHANG1 if a job of higher priority waits (NEWJOB positive), to CHECKNJ
 * if NEWJOB is negative, and straight on if it is +0 or -0. */
static void continu(void)
{
    set_A(S_PLUS_1);
    continu_plus1();
}

static void continu_plus1(void)
{
    ts(ADDRESS_OF(SKEEP5));
    switch (ccs_at(ADDRESS_OF(SKEEP6))) {
    case POSITIVE: break;
    case IS_PLUS_ZERO:
        tc_at(CONTINU + 4u); /* TC CHANG1 */
        continue_at(CHANG1);
        return;
    case NEGATIVE:
        tc_at(CONTINU + 5u); /* TC +2 */
        adrs_plus_1();
        return;
    case IS_MINUS_ZERO:
        tc_at(CONTINU + 6u); /* TC CHECKNJ */
        checknj();
        return;
    }
    switch (ccs_at(ADDRESS_OF(NEWJOB))) { /* "+1, SHOWSUM" */
    case POSITIVE:
        tc_at(CONTINU + 4u); /* TC CHANG1 */
        continue_at(CHANG1);
        return;
    case IS_PLUS_ZERO: continu_plus5(); return; /* TC +2 */
    case NEGATIVE:
        tc_at(CONTINU + 6u); /* TC CHECKNJ */
        checknj();
        return;
    case IS_MINUS_ZERO: adrs_plus_1(); return;
    }
}

/* CONTINU +5 -- LM p. 1292, CM p. 1402.
 * Back from CHANG1 too. */
void continu_plus5(void)
{
    tc_at(CONTINU + 5u); /* TC +2 */
    adrs_plus_1();
}

/* ADRS+1 -- LM p. 1292, CM p. 1402.
 * The next address of the bank, common fixed or fixed fixed. */
void adrs_plus_1(void)
{
    incr(ADDRESS_OF(SKEEP3));
    switch (ccs_at(ADDRESS_OF(SKEEP7))) {
    case POSITIVE:
        tc_at(ADRS_PLUS_1 + 2u);
        comadrs();
        return; /* TC COMADRS */
    case IS_PLUS_ZERO:
        tc_at(ADRS_PLUS_1 + 3u);
        comadrs();
        return; /* TC COMADRS */
    case NEGATIVE:
        tc_at(ADRS_PLUS_1 + 4u);
        fxadrs();
        return; /* TC FXADRS */
    case IS_MINUS_ZERO:
        tc_at(ADRS_PLUS_1 + 5u);
        fxadrs();
        return; /* TC FXADRS */
    }
}

/* NXTBNK, 17TO20, CHKSUPR, 27TO30, NXTSUPR -- LM p. 1292, CM p. 1402.
 * The next bank, until the "LAST BANK TO BE CHECKED" (LSTBNKCH), then
 * ENDSUMS ("END OF SUMMING OF BANKS."). SKEEP4 steps by 02000 a bank in
 * its bank bits; from bank 37 the step overflows ("37 TO 40 INCRMTS
 * SKEEP4 BY END RND CARRY"), and 17TO20 adds BIT15 ("SET FOR BANK 20").
 * Past bank 27 come the superbanks: bank 30 with "FIRST SUPER BANK" bits
 * 060 (27TO30), and after each superbank's last bank NXTSUPR ("SET BNK 30
 * + INCR SUPR BNK AND CANCEL", "ERC BIT OF THE 37 TO 40 ADVANCE."). */
void nxtbnk(void)
{
    cs(ADDRESS_OF(SKEEP4));
    ad(LSTBNKCH);
    if (A_is_zero()) { /* EXTEND; BZF ENDSUMS */
        continue_at(ENDSUMS);
        return;
    }
    ca(ADDRESS_OF(SKEEP4));
    ad(SBIT11);
    if (ts(ADDRESS_OF(SKEEP4))) { /* the overflow skips TC CHKSUPR: 17TO20 */
        set_A(SBIT15);
        ads(ADDRESS_OF(SKEEP4));
        tc_at(SEVENTEEN_TO20 + 2u); /* TC GONXTBNK */
        gonxtbnk();
        return;
    }
    tc_at(NXTBNK + 7u); /* TC CHKSUPR */
    mask(HI5);          /* CHKSUPR */
    if (A_is_zero()) {  /* EXTEND; BZF NXTSUPR: "INCREMENT SUPER BANK" */
        ad(SUPRCON);    /* NXTSUPR */
        ads(ADDRESS_OF(SKEEP4));
        gonxtbnk();
        return;
    }
    ad(S13BITS);                       /* 27TO30 */
    if (!A_is_zero()) {                /* EXTEND; BZF +2: "BANK SET FOR 30" */
        tc_at(TWENTY_SEVEN_TO30 + 3u); /* TC GONXTBNK */
        gonxtbnk();
        return;
    }
    set_A(SIXTY);
    ads(ADDRESS_OF(SKEEP4));
    tc_at(TWENTY_SEVEN_TO30 + 6u); /* TC GONXTBNK */
    gonxtbnk();
}

/* GONXTBNK -- LM p. 1292, CM p. 1402.
 * CCS SKEEP7 counts the banks down. Bank 00 starts it at +1 and bank 01
 * reads +0, so after bank 01 come the fixed-fixed banks: 02 (FXFX with A
 * +1, SKEEP7 -1), then 03 (A +0, SKEEP7 -0). After bank 03 common fixed
 * goes on from bank 04 with SKEEP7 BIT7, "HAS TO BE LARGER THAN NO OF
 * FXSW BANKS." */
static void gonxtbnk(void)
{
    switch (ccs_at(ADDRESS_OF(SKEEP7))) {
    case POSITIVE:
        tc_at(GONXTBNK + 1u); /* TC COMMFX */
        commfx();
        return;
    case IS_PLUS_ZERO: set_A(S_PLUS_1); /* fall through */
    case NEGATIVE:
        tc_at(GONXTBNK + 3u); /* TC FXFX */
        fxfx();
        return;
    case IS_MINUS_ZERO:
        set_A(SBIT7);
        tc_at(GONXTBNK + 5u); /* TC COMMFX */
        commfx();
        return;
    }
}

/* SOPTION -- LM pp. 1292-1293, CM pp. 1402-1403.
 * A bank's sum done: its number in L, by LEFT5 from the "BANK BITS"
 * ("BANK NUMBER BEFORE SUPER BANK"). A TS L of an overflowing A is not
 * modelled: LEFT5 returns a word. */
static void soption(void)
{
    ca(ADDRESS_OF(SKEEP4));
    mask(HI5);
    tc_at(SOPTION + 2u); /* TC LEFT5 */
    continue_at(LEFT5);
}

/* SOPTION +3, SOPT -- LM pp. 1292-1293, CM p. 1403.
 * With "SUPER BANK BITS", the bank's number is its superbank's ("SUPER
 * BANK NECESSARY", the bits shifted down by SR) plus its low three bits:
 * "BANK NUMBER WITH SUPER BANK". SHOW-BANKSUM displays it ("ON +1 GO TO
 * DISPLAY OF SUM."); ROPECHK checks it ("ON -0 CONTINUE WITH ROPE
 * CHECK."). */
void soption_plus3(void)
{
    if (ts(L_ADDRESS)) not_modelled();
    ca(ADDRESS_OF(SKEEP4));
    mask(S8BITS);
    if (!A_is_zero()) { /* EXTEND; BZF SOPT: "BEFORE SUPER BANK" */
        ts(ADDRESS_OF(SR));
        ca(L_ADDRESS);
        mask(SEVEN);
        ad_at(ADDRESS_OF(SR));
        ts(L_ADDRESS);
    }
    ca(ADDRESS_OF(SKEEP6)); /* SOPT */
    if (!A_is_zero()) {     /* EXTEND; BZF +2 */
        tc_at(SOPT + 3u);   /* TC SDISPLAY */
        continue_at(SDISPLAY);
        return;
    }
    sopt_plus4();
}

/* SOPT +4, BNKCHK -- LM p. 1293, CM p. 1403.
 * Back from SDISPLAY too. "FORCE SUM TO ABSOLUTE VALUE." (a negative sum
 * plus 1, which TS SKEEP1 can carry into an overflow that skips the CS
 * L), then the sum less the bank number ("= - BANK NUMBER") must be +1:
 * plus S-1, -1CHK ("CHECK SUM"). */
void sopt_plus4(void)
{
    switch (ccs_at(ADDRESS_OF(SKEEP1))) {
    case POSITIVE:
        tc_at(SOPT + 5u); /* TC +2 */
        ad(S_PLUS_1);
        break;
    case IS_PLUS_ZERO: tc_at(SOPT + 6u); break; /* TC +2 */
    case NEGATIVE: ad(S_PLUS_1); break;
    case IS_MINUS_ZERO: break;
    }
    if (!ts(ADDRESS_OF(SKEEP1))) cs(L_ADDRESS); /* BNKCHK */
    ad_at(ADDRESS_OF(SKEEP1));
    ad(S_MINUS_1);
    tc_at(BNKCHK + 3u); /* TC -1CHK */
    if (minus_1chk()) bnkchk_plus4();
}

void bnkchk_plus4(void)
{
    tc_at(BNKCHK + 4u); /* TC NXTBNK */
    nxtbnk();
}
