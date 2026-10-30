/* SPDX-License-Identifier: MIT */
/* Phase table maintenance: PHASE_TABLE_MAINTENANCE.agc of Luminary099 (LM,
 * listing pages 1294-1302) and Comanche055 (CM, pages 1404-1413). The
 * faithful C is c64-port/src/lm_phase_table.c and cm_phase_table.c.
 *
 * The code runs in two places: the entries in fixed-fixed memory, and the
 * work (PHSCHNG2 to BELOW4, CON2, SETUPDSP) in bank 10 ("BANK 14" in the
 * listing's pseudo-addresses), reached and left by DTCB. ONEORTWO and the
 * routines after it are back in fixed-fixed: with the caller's bank put
 * back for the moment, they can read the words after the TC PHASCHNG.
 *
 * Every word of memory and every register ends as the original leaves it
 * (STYLE.md, Fidelity). Each entry ends where the faithful port's routine
 * does: at BANKJUMP, at NOVAC, or at a DTCB. */
#include "phase_table.h"
#include "../agc/names.h"

static void phasjump(void);
static void below1(void);
static void below2(void);
static void oneortwo(void);
static void checkb(void);
static void oldprio(void);
static void getprio(void);
static void con1_minus1(void);
static void getnewnm(void);
static void tocon2(void);

/* INDEX Q; CA 0; INCR Q: the word after the TC, and on past it. */
static void next_word_after_tc(void)
{
    set_A(read_word(Q));
    Q = add16(Q, 1u);
    correct_L();
}

/* MASK OCT7; DOUBLE: a phase word's restart group, doubled, the offset of
 * the group's words in the tables (OF_GROUP). */
static void group_times_two(void)
{
    mask(OCT7);
    double_A();
}

/* MASK OCT17770; EXTEND; MP BIT12: a phase word's phase, bits 4-10,
 * shifted down three places. */
static void phase_of(void)
{
    mask(OCT17770);
    mp(BIT12);
}

/* ======================================================================
 * The major mode
 * ====================================================================== */

/* NEWMODEX -- LM p. 1294, CM p. 1404.
 * "UPDATE MODREG. ENTRY FOR MODE IN FIXED": the new major mode follows the
 * TC NEWMODEX. */
void newmodex(void)
{
    next_word_after_tc();
    newmodea();
}

/* NEWMODEA -- LM p. 1294, CM p. 1404.
 * "ENTRY FOR MODE IN A." */
void newmodea(void)
{
    ts(ADDRESS_OF(MODREG));
    mmdsplay();
}

/* MMDSPLAY and PREBJUMP -- LM p. 1294, CM p. 1404.
 * "DISPLAY MAJOR MODE": go to SETUPDSP, in bank 10, through BANKJUMP, with
 * the caller's return and banks as a 2CADR in A and L: PREBJUMP "PUTS
 * BBANK IN L", and BANKJUMP's XCH Q "PUTS Q INTO A". */
void mmdsplay(void)
{
    set_A(SETUPDSP_CADR); /* CAF +3: CADR SETUPDSP */
    lxch(BBANK_ADDRESS);
    continue_at(BANKJUMP);
}

/* CHECKMM -- LM p. 1294, CM p. 1404.
 * "RETURN TO CALLER +3 IF MODE = THAT AT CALLER +1. OTHERWISE RETURN TO
 * CALLER +2." The returns are the fixed-fixed routines Q+2 and Q+1. */
void checkmm(void)
{
    set_A(negate(read_word(Q))); /* INDEX Q; CS 0 */
    A = add16(A, sign_extend(MODREG));
    correct_L();
    continue_at(A_is_zero() ? Q_PLUS_2 : Q_PLUS_1); /* EXTEND; BZF Q+2; TCF Q+1 */
}

/* SETUPDSP -- LM p. 1294, CM p. 1404.
 * Start the job DSPMMJOB, which shows the major mode, from a job or a task
 * ("EITHER A TASK OR JOB CAN COME TO NEWMODE X"). The caller's return
 * 2CADR waits in RUPTREG1 meanwhile. */
void setupdsp(void)
{
    inhint();
    dxch(ADDRESS_OF(RUPTREG1)); /* "SAVE CALLER'S RETURN 2CADR" */
    set_A(PRIO30);
    tc_at((word)(SETUPDSP + 3u)); /* TC NOVAC; 2CADR DSPMMJOB */
    continue_at(NOVAC);
}

/* SETUPDSP, after NOVAC returns -- LM p. 1294, CM p. 1404. */
void setupdsp_after_novac(void)
{
    dxch(ADDRESS_OF(RUPTREG1));
    relint();
    dtcb((word)(SETUPDSP + 8u)); /* DXCH Z: "RETURN" */
}

/* ======================================================================
 * Phase changes: the entries
 * ====================================================================== */

/* 2PHSCHNG -- LM p. 1298, CM p. 1409.
 * Two phase changes in one call, "TO START UP A GROUP OR CHANGE A GROUP
 * WHILE UNDER THE CONTROL OF A DIFFERENT GROUP". The first word after the
 * TC is a type A phase word: its phase goes to TEMPP2, its group (doubled)
 * to TEMPG2, its T bit (set TBASE) to TEMPSW2. The second, of any type,
 * goes on as PHASCHNG's word. (The CM gets there by TCF PHASCHNG +3; the
 * LM repeats those three instructions.) */
void two_phschng(void)
{
    inhint();
    next_word_after_tc();
    ts(ADDRESS_OF(TEMPP2));
    group_times_two();
    ts(ADDRESS_OF(TEMPG2));
    set_A(TEMPP2);
    phase_of(); /* "NEED ONLY 1770, BUT WHY GET A NEW CONST." */
    xch(ADDRESS_OF(TEMPP2));
    mask(BIT15);
    ts(ADDRESS_OF(TEMPSW2)); /* "INDICATES WHETHER TO SET TBASE OR NOT" */
    next_word_after_tc();
    ts(ADDRESS_OF(TEMPSW));
    phasjump();
}

/* PHASCHNG -- LM p. 1298, CM p. 1409.
 * "THE MAIN WAY OF MAKING PHASE CHANGES FOR RESTARTS": the phase word
 * follows the TC PHASCHNG. It goes to TEMPSW, and TEMPSW2 = 1 marks a
 * single change. */
void phaschng(void)
{
    inhint();
#if LUMINARY099
    next_word_after_tc();
    phschnga();
#else
    set_A(ONE);
    ts(ADDRESS_OF(TEMPSW2));
    next_word_after_tc();
    ts(ADDRESS_OF(TEMPSW));
    phasjump();
#endif
}

#if LUMINARY099
/* PHSCHNGA -- LM p. 1298 (the LM only).
 * PHASCHNG with the phase word in A: "FIRST OCTAL PARAMETER IN A." */
void phschnga(void)
{
    inhint();
    ts(ADDRESS_OF(TEMPSW));
    set_A(ONE);
    ts(ADDRESS_OF(TEMPSW2));
    phasjump();
}
#endif

/* PHASJUMP -- LM p. 1299, CM p. 1410 (unlabelled in the CM).
 * "OFF TO SWITCHED BANK": DTCB to PHSCHNG2 in bank 10. L comes back with
 * the caller's BBANK, which PHSCHNG2 keeps in TEMPBBCN. */
static void phasjump(void)
{
    L = sign_extend(ADRPCHN2[1]); /* EXTEND; DCA ADRPCHN2 */
    A = sign_extend(ADRPCHN2[0]);
    dtcb((word)(ADRPCHN2_ADDRESS - 1u));
}

/* ======================================================================
 * Phase changes: the work
 * ====================================================================== */

/* PHSCHNG2 -- LM p. 1300, CM p. 1411.
 * Take the phase word in TEMPSW apart: TEMPG gets the group (doubled),
 * TEMPP the phase, TEMPSW the T and L bits (set TBASE, set LONGBASE). A
 * type A word (a fixed phase) is stored here. Types B and C, which name a
 * job, task or LONGCALL in the call itself (bit 13 or 12), go to ONEORTWO
 * for the words after the TC. */
void phschng2(void)
{
    lxch(ADDRESS_OF(TEMPBBCN)); /* the caller's BBANK */
    set_A(TEMPSW);
    group_times_two();
    ts(ADDRESS_OF(TEMPG));
    set_A(TEMPSW);
    phase_of();
    ts(ADDRESS_OF(TEMPP));
    set_A(TEMPSW);
    mask(OCT60000);
    xch(ADDRESS_OF(TEMPSW));
    mask(OCT14000);
    /* CCS A: never negative, as A is masked. */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        oneortwo();
        return;
    }
    set_A(TEMPP); /* "START STORING THE PHASE INFORMATION" */
    ts(OF_GROUP(PHASE1, TEMPG));
    below1();
}

/* BELOW1 -- LM p. 1301, CM p. 1412.
 * "IS IT A PHASCHNG OR A 2PHSCHNG": for a 2PHSCHNG, store its first
 * (type A) phase and its complement, and set that group's TBASE if asked. */
static void below1(void)
{
    switch (ccs(TEMPSW2)) {
    case POSITIVE: below2(); return;      /* "IT'S A PHASCHNG" */
    case IS_PLUS_ZERO:                    /* "IT'S A 2PHSCHNG" (TCF +1) */
    case NEGATIVE: set_A(negate(TEMPP2)); /* fall through */
    case IS_MINUS_ZERO: break;
    }
    lxch(ADDRESS_OF(TEMPP2));
    dxch(OF_GROUP(MPHASE1, TEMPG2)); /* -PHASE and PHASE of the first group */
    switch (ccs(TEMPSW2)) {
    case POSITIVE: /* NOOP: "CAN'T GET HERE" */
    case IS_PLUS_ZERO: below2(); return;
    case NEGATIVE: set_A(negate(TIME1)); /* fall through */
    case IS_MINUS_ZERO: break;
    }
    ts(OF_GROUP(TBASE1, TEMPG2));
    below2();
}

/* BELOW2, BELOW3 and BELOW4 -- LM p. 1301, CM p. 1412.
 * "SEE IF WE SHOULD SET TBASE OR LONGBASE", and set them from the clock;
 * then "STORE THE FINAL PART OF THE PHASE", its complement, which a restart
 * checks the phase against. Then RELINT, and DTCB back to the caller after
 * the words that followed the TC. TEMPSW holds T (040000) and L (020000). */
static void below2(void)
{
    bool set_longbase = false;

    switch (ccs(TEMPSW)) {
    case POSITIVE: set_longbase = true; break; /* "SET LONGBASE ONLY" */
    case IS_PLUS_ZERO: break;                  /* "SET NEITHER" */
    case NEGATIVE:
        set_A(negate(TIME1)); /* "SET TBASE TO BEGIN WITH" */
                              /* fall through */
    case IS_MINUS_ZERO:
        ts(OF_GROUP(TBASE1, TEMPG));
        set_A(TEMPSW); /* "SHALL WE NOW SET LONGBASE": T plus BIT14COM */
        A = add16(A, sign_extend(BIT14COM));
        correct_L();
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: /* NOOP, then the constant BIT14COM as code */
        case IS_PLUS_ZERO: /* "***** CAN'T GET HERE *****" */ continue_at(BIT14COM_ADDRESS); return;
        case NEGATIVE: break; /* "NO WE NEED ONLY SET TBASE" */
        case IS_MINUS_ZERO: set_longbase = true; break;
        }
        break;
    }
    if (set_longbase) { /* BELOW3 */
        dca(ADDRESS_OF(TIME2));
        dxch(ADDRESS_OF(LONGBASE));
    }
    /* BELOW4 */
    set_A(negate(TEMPP));
    ts(OF_GROUP(MPHASE1, TEMPG));
    A = Q; /* CA Q: the return, after the words that followed the TC */
    lxch(ADDRESS_OF(TEMPBBCN));
    relint();
    dtcb((word)(BELOW4 + 6u));
}

/* ONEORTWO -- LM p. 1299, CM p. 1410.
 * For a type B or C phase word: back to the caller's banks for a moment,
 * then gather the priority (or delta time) and the address to restart at,
 * into TEMPPR, TEMPNM and TEMPBB. A is what PHSCHNG2's CCS left: the
 * word's bits 13-12, less one. */
static void oneortwo(void)
{
    lxch(ADDRESS_OF(TEMPBBCN));
    lxch(BBANK_ADDRESS); /* the caller's banks; bank 10's to TEMPBBCN */
    lxch(ADDRESS_OF(TEMPBBCN));
    mask(OCT14000); /* "SEE WHAT KIND OF PHASE CHANGE IT IS" */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        checkb(); /* "IT IS OF TYPE `B'." */
        return;
    }
    set_A(TEMPP);
    mask(BIT7);
    if (ccs_register(A_ADDRESS) == POSITIVE) /* "SHALL WE USE THE OLD PRIORITY" */
        getprio();                           /* "NO GET A NEW PRIORITY (OR DELTA T)" */
    else
        oldprio();
}

/* CHECKB -- LM p. 1302, CM p. 1413.
 * A type B word: bit 12 asks for a new priority. */
static void checkb(void)
{
    mask(BIT12);
    if (ccs_register(A_ADDRESS) == POSITIVE)
        getprio(); /* "IT IS, SO GET NEW PRIORITY" */
    else
        oldprio(); /* "IT ISN'T, USE THE OLD PRIORITY." */
}

/* OLDPRIO -- LM p. 1299, CM p. 1410.
 * "USE THE OLD PRIORITY (OR DELTA T)", the group's PHSPRDT. */
static void oldprio(void)
{
    set_A(PHSPRDT1_OF(TEMPG));
    con1_minus1();
}

/* GETPRIO -- LM p. 1299, CM p. 1410.
 * The priority or delta time follows the phase word ("DON'T CARE IF DIRECT
 * OR INDIRECT, LEAVE THAT DECISION TO RESTARTS"). */
static void getprio(void)
{
    next_word_after_tc();
    con1_minus1();
}

/* CON1 -1 and CON1 -- LM p. 1299, CM p. 1410.
 * Keep the priority; then "SEE IF A 2CADR IS GIVEN". If not, the restart
 * address is the return, in the caller's banks and superbank. */
static void con1_minus1(void)
{
    ts(ADDRESS_OF(TEMPPR));
    set_A(TEMPP);
    mask(BIT8);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        getnewnm();
        return;
    }
    A = Q; /* CA Q */
    ts(ADDRESS_OF(TEMPNM));
    set_A(BBANK);
    ror_superbnk(); /* "PICK UP USER'S SUPERBANK" */
    ts(ADDRESS_OF(TEMPBB));
    tocon2();
}

/* GETNEWNM -- LM p. 1300, CM p. 1410.
 * The 2CADR to restart at follows; the return is after it. */
static void getnewnm(void)
{
    L = sign_extend(read_word((word)(Q + 1u))); /* EXTEND; INDEX Q; DCA 0 */
    A = sign_extend(read_word(Q));
    dxch(ADDRESS_OF(TEMPNM));
    set_A(TWO);
    A = add16(A, Q); /* ADS Q: "OBTAIN RETURN ADDRESS" */
    Q = A;
    correct_L();
    tocon2();
}

/* TOCON2 -- LM p. 1299, CM p. 1410.
 * "BACK TO SWITCHED BANK": DTCB to CON2 in bank 10. */
static void tocon2(void)
{
    set_A(CON2ADR);
    lxch(ADDRESS_OF(TEMPBBCN));
    dtcb((word)(TOCON2 + 2u));
}

/* CON2 -- LM p. 1302, CM p. 1412.
 * Store what ONEORTWO gathered, the group's phase, priority and 2CADR, and
 * finish as BELOW1. */
void con2(void)
{
    lxch(ADDRESS_OF(TEMPBBCN));
    set_A(TEMPP);
    ts(OF_GROUP(PHASE1, TEMPG));
    set_A(TEMPPR);
    ts(OF_GROUP(PHSPRDT1, TEMPG));
    dca(ADDRESS_OF(TEMPNM));
    dxch(OF_GROUP(PHSNAME1, TEMPG));
    below1();
}
