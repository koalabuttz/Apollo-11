/* SPDX-License-Identifier: MIT */
/* Down-telemetry: DOWN-TELEMETRY_PROGRAM.agc of Luminary099 (LM, listing
 * pages 988-997) and Comanche055 (CM, pages 1093-1102). The faithful C is
 * c64-port/src/lm_downtelemetry.c and cm_downtelemetry.c.
 *
 * The two missions run the same code, but for one thing: at the start of
 * each DOWNRUPT the CM sets the word order bit (bit 7 of channel 13) every
 * time, the LM (through WOTEST) only when it is clear.
 *
 * How the program reads its lists. A downlist is all words in bank 05:
 *   - DNTABLE +n is the address of downlist n's control list;
 *   - a control list holds DNADRs, DNCHANs and DNPTRs; its last word is
 *     stored complemented (negative), which ends the list;
 *   - an nDNADR is an ECADR with n-1 in bits 12-14 (n double words from
 *     there), a DNCHAN an ECADR with 0111 there, and a DNPTR the
 *     instruction CA <sublist>;
 *   - a sublist holds DNADRs and DNCHANs, its last one negative too. A
 *     snapshot sublist is one whose first word is negative.
 * The program reads a list word with INDEX: "INDEX CTLIST; CA 0" runs the
 * instruction CA <the address in CTLIST>, and "INDEX SUBLIST; 0 0" (the
 * word 00000) the instruction that SUBLIST holds, a DNPTR's CA. Reading
 * channels works the same way: "INDEX DNECADR; INDEX 0 -4000" adds the
 * DNCHAN to an INDEX word and runs, under EXTEND, READ <channel>.
 *
 * A CCS continues at one of the four words after it, for a positive, +0,
 * negative or -0 value; the code lets each case run on into the next as
 * the listing does, and some of those words are constants that the AGC
 * then runs as instructions. The listing marks most of them "can't get
 * here"; each is written out where it happens.
 *
 * The binding (src/binding/downtelemetry_routines.c) gives the routines
 * the fixed memory they read (the downlists and the constants) and the
 * channels. A list word that would make some other instruction, or a
 * channel the program does not use, is not modelled: not_modelled(). */
#include "downtelemetry.h"
#include "../agc/names.h"

#define CA_CODE    3u      /* CA's operation code */
#define INDEX_WORD 050000u /* INDEX 0 */
#define READ_LAST  000777u /* READ <channel>: the highest such word */

static void tc_dntmgoto(word at);
static void newlist(void);
static void nextincl(void);
static void setwo(void);
static void setwo_plus1(void);
static void setwo_plus2(void);
static void dodnchan(void);
static void wozero(void);
static void dodnptr(void);
static void snaploop(void);
static void fetch2wd(void);
static void dosublst(void);
static void nextinsl(void);
static void dntmexit(void);
static void dndumpi_plus2(void);
static void dndump2(void);
static void sendid(void);
static void sendid_plus3(void);

/* INDEX k; <instruction>, where the downlists make the sum a CA: CA of the
 * list word the sum names, in fixed memory with the lists. */
static void index_ca(word instruction, word index)
{
    word sum = indexed_instruction(instruction, index);
    if (OPERATION_CODE(sum) != CA_CODE || ADDRESS_FIELD(sum) < 02000u) not_modelled();
    ca(ADDRESS_FIELD(sum));
}

/* EXTEND; INDEX DNECADR; INDEX 0 -400n: the DNCHAN plus the INDEX word is
 * the extended instruction READ <channel>. */
static void read_dnchan(word index_word)
{
    word sum = indexed_instruction(index_word, DNECADR);
    if (sum > READ_LAST) not_modelled();
    read_channel(sum);
}

/* ======================================================================
 * Each DOWNRUPT
 * ====================================================================== */

/* DODOWNTM -- LM p. 991, CM p. 1096.
 * The DOWNRUPT interrupt comes here with the interrupted BBANK in A. Save
 * it and Q, set the word order bit, and go where DNTMGOTO says. */
void dodowntm(void)
{
    ts(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
#if LUMINARY099
    /* TCF WOTEST */
    set_A(BIT7);
    rand_channel(CHAN13);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: /* already set */ tc_dntmgoto(WOTEST + 4u); return;
    case IS_PLUS_ZERO: set_A(BIT7); break;
    case NEGATIVE: break;               /* TCF WO1 */
    case IS_MINUS_ZERO: not_modelled(); /* RAND of BIT7 is never -0 */
    }
    /* WO1 */
    wor_channel(CHAN13);
    tc_dntmgoto(WO1 + 2u);
#else
    set_A(BIT7);
    wor_channel(CHAN13);
    tc_dntmgoto(DODOWNTM + 6u);
#endif
}

/* TC DNTMGOTO at at: Q gets the return, and the AGC goes to DNTMGOTO, an
 * erasable word, and runs it (dntmgoto()). */
static void tc_dntmgoto(word at)
{
    tc_at(at);
    continue_at(ADDRESS_OF(DNTMGOTO));
}

/* DNTMGOTO -- LM p. 991, CM p. 1096 (TC DNTMGOTO).
 * The word there is an address in bank 05, which the AGC runs as the
 * instruction TC <address>. */
void dntmgoto(void)
{
    tc_through(ADDRESS_OF(DNTMGOTO));
}

/* ======================================================================
 * The downlist
 * ====================================================================== */

/* DNPHASE1 -- LM p. 991, CM p. 1096.
 * Start the downlist again from its first word: from the next DOWNRUPT on,
 * DNPHASE2 sends it. */
void dnphase1(void)
{
    set_A(NEGONE);
    ts(ADDRESS_OF(SUBLIST));
    ts(ADDRESS_OF(DNECADR));
    set_A(LDNPHAS2);
    ts(ADDRESS_OF(DNTMGOTO));
    newlist();
}

/* DNPHASE2 -- LM p. 991, CM p. 1096.
 * Send the next two words: more of a DNADR still being sent (DNECADR
 * positive), else the sublist's next word if one is in control (SUBLIST
 * positive), else the control list's next word, or a new list at its end
 * (CTLIST negative). */
void dnphase2(void)
{
    switch (ccs_at(ADDRESS_OF(DNECADR))) {
    case POSITIVE: /* DODNADR */
        tc_at(DODNADR);
        fetch2wd();
        return;
    case IS_PLUS_ZERO:
        mask_at(ADDRESS_FIELD(MINTIME2)); /* MINTIME2 runs as MASK 7753 */
        break;                            /* TCF +1 */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(ADDRESS_OF(SUBLIST))) {
    case POSITIVE: nextinsl(); return;
    case IS_PLUS_ZERO:
        mask_at(ADDRESS_FIELD(DNADRDCR)); /* DNADRDCR runs as MASK 4001 */
    /* fall through */
    case NEGATIVE: ca(ADDRESS_OF(CTLIST)); break; /* CHKLIST */
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero_or_minus())
        newlist();
    else
        nextincl();
}

/* NEWLIST -- LM p. 992, CM p. 1097.
 * Point CTLIST at downlist DNLSTCOD's control list, and send the list's ID
 * (-DNLSTCOD) with the sync bits. */
static void newlist(void)
{
    index_ca((word)(CA_CODE << 12 | DNTABLE), DNLSTCOD);
    ts(ADDRESS_OF(CTLIST));
    cs(ADDRESS_OF(DNLSTCOD));
    sendid_plus3();
}

/* NEXTINCL -- LM p. 992, CM p. 1097.
 * Take the control list's next word into DNECADR and step CTLIST on; the
 * last word (stored negative) leaves CTLIST negative, its complement. A
 * 1DNADR of TIME2 first sets the word order code to zero. */
static void nextincl(void)
{
    index_ca((word)(CA_CODE << 12), CTLIST); /* INDEX CTLIST; CA 0 */
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: incr(ADDRESS_OF(CTLIST)); break; /* TCF +4 */
    case IS_PLUS_ZERO: break;
    case NEGATIVE: xch(ADDRESS_OF(CTLIST)); /* fall through */
    case IS_MINUS_ZERO:
        com();
        xch(ADDRESS_OF(CTLIST));
        break;
    }
    incr(A_ADDRESS); /* +4: the word again (CCS took one off) */
    ts(ADDRESS_OF(DNECADR));
    ad(MINTIME2);
    switch (ccs_register(A_ADDRESS)) {
    case IS_PLUS_ZERO:
        cs(ADDRESS_FIELD(MINB1314)); /* MINB1314 runs as CS 7777 */
    /* fall through */
    case POSITIVE:
    case NEGATIVE: setwo_plus1(); return;
    case IS_MINUS_ZERO: setwo(); return; /* TIME2 */
    }
}

/* SETWO -- LM p. 992, CM p. 1097.
 * Set the word order code to zero; then send the word in DNECADR as what
 * it is: a DNADR (bits 13-14 not both set), a DNPTR (bits 12-14 not all
 * set) or a DNCHAN. */
static void setwo(void)
{
    tc_at(SETWO);
    wozero();
    setwo_plus1();
}

static void setwo_plus1(void)
{
    ca(ADDRESS_OF(DNECADR));
    setwo_plus2();
}

static void setwo_plus2(void)
{
    ad(MINB1314);
    if (A_is_zero_or_minus()) {
        fetch2wd();
        return;
    }
    ad(MINB12);
    if (A_is_zero_or_minus()) {
        dodnptr();
        return;
    }
    dodnchan();
}

/* DODNCHAN -- LM p. 992, CM p. 1097.
 * Send two channels: the DNCHAN's channel +1 in A, the channel itself in L
 * (each TC 6 is EXTEND). DNECADR goes back to -1. */
static void dodnchan(void)
{
    read_dnchan((word)(INDEX_WORD - 04000u));
    ts(L_ADDRESS);
    read_dnchan((word)(INDEX_WORD - 04001u));
    ts(ADDRESS_OF(DNECADR));
    set_A(NEGONE);
    xch(ADDRESS_OF(DNECADR));
    dntmexit();
}

/* WOZERO -- LM pp. 992-993, CM pp. 1097-1098.
 * Set the word order code (bit 7 of channel 13) to zero. */
static void wozero(void)
{
    set_A(negate(BIT7)); /* CS BIT7 */
    wand_channel(CHAN13);
    tc_through(Q_ADDRESS);
}

/* DODNPTR -- LM p. 993, CM p. 1098.
 * The DNPTR in DNECADR points to a sublist: a regular one goes to SUBLIST
 * word by word (DOSUBLST); a snapshot one (its first word negative) is
 * copied at once, below. */
static void dodnptr(void)
{
    index_ca(0u, DNECADR); /* INDEX DNECADR; 0 0: the DNPTR's own CA */
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        ca(ADDRESS_OF(DNECADR));
        dosublst();
        return;
    case IS_PLUS_ZERO: dosublst(); return;
    case NEGATIVE: xch(ADDRESS_OF(DNECADR)); break;
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(SUBLIST));
    set_A(ZERO);
    xch(ADDRESS_OF(TMINDEX));
    snaploop();
}

/* SNAPLOOP -- LM p. 993, CM p. 1098.
 * Copy the snapshot, the second word of the sublist on, into DNTMBUFF, a
 * double word for each 1DNADR, then send the first one. A holds each
 * 1DNADR less one (CCS took one off): its EBANK bits go to EBANK, and DCA
 * 1401 + its low eight bits reads the 1DNADR's double word. */
static void snaploop(void)
{
    enum sign next;

    do {
        ts(EBANK_ADDRESS);
        mask(LOW8);
        dca(indexed(01401u, a_word()));
        dxch(indexed(ADDRESS_OF(DNTMBUFF), TMINDEX));
        incr(ADDRESS_OF(TMINDEX));
        incr(ADDRESS_OF(TMINDEX));
        /* SNAPAGN */
        incr(ADDRESS_OF(SUBLIST));
        index_ca(0u, SUBLIST); /* INDEX SUBLIST; 0 0 */
        next = ccs_register(A_ADDRESS);
    } while (next == POSITIVE);

    switch (next) {
    case IS_PLUS_ZERO: /* LDNPHAS2 (GENADR DNPHASE2) runs as TC DNPHASE2 */
        tc_at(ADDRESS_OF(LDNPHAS2));
        dnphase2();
        return;
    case NEGATIVE: ts(ADDRESS_OF(SUBLIST)); break; /* the last word */
    default: break;
    }
    set_A(NEGONE);
    ts(ADDRESS_OF(DNECADR));
    xch(ADDRESS_OF(SUBLIST));
    ts(EBANK_ADDRESS);
    mask(LOW8);
    dca(indexed(01401u, a_word()));
    dntmexit(); /* SNAPEND */
}

/* FETCH2WD -- LM p. 994, CM p. 1099.
 * Send the next double word of the DNADR in DNECADR, and count it off:
 * DNADRDCR takes one from the count in bits 12-14 and moves the address
 * two on. */
static void fetch2wd(void)
{
    ca(ADDRESS_OF(DNECADR));
    ts(EBANK_ADDRESS);
    mask(LOW8);
    ts(L_ADDRESS);
    set_A(DNADRDCR);
    ads(ADDRESS_OF(DNECADR));
    dca(indexed(01400u, overflow_corrected(L))); /* EXTEND; INDEX L; DCA 1400 */
    dntmexit();
}

/* DOSUBLST, NEXTINSL -- LM p. 994, CM p. 1099.
 * Take the sublist's next word into DNECADR and send it; the last one
 * (negative) leaves SUBLIST at -1. */
static void dosublst(void)
{
    ts(ADDRESS_OF(SUBLIST));
    nextinsl();
}

static void nextinsl(void)
{
    index_ca(0u, SUBLIST); /* INDEX SUBLIST; 0 0 */
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: incr(ADDRESS_OF(SUBLIST)); break; /* TCF +4 */
    case IS_PLUS_ZERO: break;
    case NEGATIVE: ts(ADDRESS_OF(SUBLIST)); /* fall through */
    case IS_MINUS_ZERO:
        set_A(NEGONE);
        xch(ADDRESS_OF(SUBLIST));
        break;
    }
    incr(A_ADDRESS); /* +4 */
    ts(ADDRESS_OF(DNECADR));
    setwo_plus2();
}

/* DNTMEXIT, TMEXITL, TMRESUME -- LM p. 994, CM p. 1099.
 * Send A and L in channels 34 and 35, and end the interrupt. */
static void dntmexit(void)
{
    write_channel(DNTM1);
    ca(L_ADDRESS);
    write_channel(DNTM2);
    continue_at(RESUME);
}

/* ======================================================================
 * The erasable dump
 * ====================================================================== */

/* DNDUMPI -- LM p. 996, CM p. 1101.
 * Start the dump at E-bank 0. Each bank starts with four words: ERASID and
 * the sync bits, then DUMPLOC (the dump's count and the bank) and TIME1. */
void dndumpi(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DUMPLOC));
    dndumpi_plus2();
}

static void dndumpi_plus2(void)
{
    tc_at(DNDUMPI + 2u);
    sendid();
}

/* DNDUMPI +3 -- LM p. 996, CM p. 1101.
 * SENDID left DNTMGOTO here: send DUMPLOC and TIME1. */
void dndumpi_plus3(void)
{
    set_A(LDNDUMP1);
    ts(ADDRESS_OF(DNTMGOTO));
    ca(ADDRESS_OF(TIME1));
    xch(L_ADDRESS);
    ca(ADDRESS_OF(DUMPLOC));
    dntmexit();
}

/* DNDUMP -- LM p. 996, CM p. 1101.
 * The next double word. At the end of a bank: at the end of the dumps
 * DUMPCNT asked for (its bit 12, 13 or 14 reached in DUMPLOC), back to the
 * downlist (DNPHASE1); else the next bank. */
void dndump(void)
{
    set_A(TWO);
    ads(ADDRESS_OF(DUMPLOC));
    mask(LOW8);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: dndump2(); return;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(DUMPLOC));  /* fall through */
    case NEGATIVE: mask_at(ADDRESS_OF(DUMPCNT)); /* fall through */
    case IS_MINUS_ZERO: mask(PRIO34); break;
    }
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: dnphase1(); return;
    case IS_PLUS_ZERO: dndumpi_plus2(); return;
    case NEGATIVE: dndump1(); return;
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(DNTMGOTO));
        dndump2();
        return;
    }
}

/* DNDUMP1 -- LM p. 996, CM p. 1101.
 * After a bank's first four words: DNDUMP sends the rest. */
void dndump1(void)
{
    set_A(LDNDUMP);
    ts(ADDRESS_OF(DNTMGOTO));
    dndump2();
}

/* DNDUMP2 -- LM p. 996, CM p. 1101.
 * Send the double word at DUMPLOC: EBANK from it, the address in the bank
 * in Q. MASK reads the words, so the editing registers (020-023) are read
 * without being changed. Dumping E-bank 0 reads the central registers
 * too: Z reads as the address after the MASK being run. */
static void dndump2(void)
{
    ca(ADDRESS_OF(DUMPLOC));
    ts(EBANK_ADDRESS);
    mask(LOW8);
    ts(Q_ADDRESS);
    set_A(NEG0);
    ts(L_ADDRESS);
    continue_at(DNDUMP2 + 8u);
    mask_at(indexed(01401u, Q)); /* INDEX Q; MASK 1401 */
    xch(L_ADDRESS);
    continue_at(DNDUMP2 + 11u);
    mask_at(indexed(01400u, Q)); /* INDEX Q; MASK 1400 */
    dntmexit();
}

/* SENDID -- LM pp. 996-997, CM pp. 1101-1102.
 * Send the dump's ID word with the sync bits, and set DNTMGOTO so the next
 * DOWNRUPT goes on after the caller's TC SENDID. NEWLIST comes in at
 * SENDID +3 with the downlist's ID in A. */
static void sendid(void)
{
    qxch(ADDRESS_OF(DNTMGOTO));
    set_A(ERASID);
    sendid_plus3();
}

static void sendid_plus3(void)
{
    ts(L_ADDRESS);
    tc_at(SENDID + 4u);
    wozero();
    set_A(LOWIDCOD);
    xch(L_ADDRESS);
    dntmexit();
}
