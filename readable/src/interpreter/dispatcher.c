/* SPDX-License-Identifier: MIT */
/* The interpreter's dispatcher: INTERPRETER.agc section 1 (LM pp.
 * 1002-1013, CM pp. 1107-1118). The faithful C is c64-port/src/
 * lm_interp_core.c (run(), lmi_newops, lmi_direct, lmi_indexed, pushup,
 * lmi_misc, lmi_unary); the routines below say the same thing label by
 * label.
 *
 * How an order pair is taken apart. NEWOPS reads the word at LOC. A pair is
 * stored complemented, so it reads negative, and CCS A turns it back into
 * the two seven-bit codes: the left one is run now and the right one waits
 * in EDOP, where DANZIG finds it next time. Each code's low two bits are
 * its prefix:
 *   01 or 11  an order with an address (INDJUMP, bits 3-7 its number),
 *             11 when the address is indexed;
 *   10        an index, branch or other miscellaneous order (MISCJUMP),
 *             with a 15-bit address in the next word;
 *   00        a unary order (UNAJUMP) or a short shift; 00 0000 0 is EXIT.
 * OPJUMP tests the prefix bits by cycling the code through CYR, the
 * editing register that rotates a word right each time it is read. */
#include "interpreter.h"
#include "../agc/names.h"

static void opjump(void);
static void opjump2(void);
static void opjump3(void);
static void ierastst(void);
static void index_(void);
static void index2(void);
static void pushup(void);
static void regup_plus2(void);
static void itr(word instruction);

/* ======================================================================
 * Entering and leaving interpretive mode
 * ====================================================================== */

/* INTPRET -- LM p. 1002, CM p. 1107.
 * Basic code enters the interpreter with TC INTPRET; the interpretive
 * program starts in the word after the TC. "INTPRET SETS LOC TO THE FIRST
 * INSTRUCTION, BANKSET TO THE BBANK OF THE OBJECT INTERPRETIVE PROGRAM, AND
 * INTBIT15 TO THE BIT15 CONTENTS OF FBANK." */
void intpret(void)
{
    relint();
    intpret_plus2(); /* (EXTEND) */
}

/* INTPRET +2: "SET LOC TO THE WORD FOLLOWING THE TC" (EXTEND; QXCH LOC). */
void intpret_plus2(void)
{
    qxch(ADDRESS_OF(LOC));
    intpret_plus3();
}

/* INTPRET +3 -- "INTERPRETIVE BRANCHES FINISH HERE." The program's bank
 * becomes the job's (BANKSET), and its superbank bit is kept in INTBIT15
 * for the addresses its orders give. */
void intpret_plus3(void)
{
    ca(BBANK_ADDRESS);
    ts(ADDRESS_OF(BANKSET));
    mask(BIT15);
    ts(ADDRESS_OF(INTBIT15));
    ts(ADDRESS_OF(EDOP)); /* "MAKE SURE NO INSTRUCTIONS LEFT OVER" */
    continue_at(NEWOPS);
}

/* INTRSM -- LM p. 1002, CM p. 1107.
 * "RESUME SUSPENDED INTERPRETIVE JOB": L holds the job's BBANK. */
void intrsm(void)
{
    lxch(BBANK_ADDRESS);
    intpret_plus3();
}

/* DLOAD -- LM p. 1002, CM p. 1107.
 * "DLOAD LOADS MPAC, MPAC +1, LEAVING ZERO IN MPAC +2." */
void dload(void)
{
    dca(indexed(0u, ADDRWD));
    sload2();
}

/* SLOAD2 -- LM p. 1002, CM p. 1107.
 * The end of DLOAD and SLOAD: A,L into MPAC, MPAC +2 cleared, and the mode
 * declared double precision. */
void sload2(void)
{
    dxch(ADDRESS_OF(MPAC));
    set_A(ZERO);
    sload2_plus2();
}

void sload2_plus2(void)
{
    ts(ADDRESS_OF(MPAC) + 2u); /* "AND DECLARE DP MODE" */
    newmode();
}

/* NEWMODE -- LM p. 1003, CM p. 1108.
 * "PROLOGUE FOR MODE-CHANGING INSTRUCTIONS": A is the new MODE. The order
 * is done; DANZIG takes the next one. */
void newmode(void)
{
    ts(ADDRESS_OF(MODE));
    continue_at(DANZIG);
}

/* ======================================================================
 * Fetching orders
 * ====================================================================== */

/* DANZIG -- LM p. 1003, CM p. 1108.
 * "AT THE END OF MOST INSTRUCTIONS, CONTROL IS GIVEN TO DANZIG TO DISPATCH
 * THE NEXT OPERATION." The banks go back to the program's first, "SO THAT
 * IT MAY BE SAVED DIRECTLY BY CHANJOB". */
void danzig(void)
{
    ca(ADDRESS_OF(BANKSET));
    ts(BBANK_ADDRESS);
    noibnksw();
}

/* NOIBNKSW -- LM p. 1003, CM p. 1108.
 * DANZIG without the bank setting (SETPD ends here). The order left in
 * EDOP runs first ("EDOP IS SET TO ZERO ON ITS RE-EDITING": CCS reads it
 * once more through the editing register, which leaves +0). With none left,
 * a waiting job of higher priority (NEWJOB positive) takes the computer;
 * otherwise LOC moves to the next word. */
void noibnksw(void)
{
    switch (ccs_at(ADDRESS_OF(EDOP))) {
    case POSITIVE: opjump(); return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: continue_at(CHANG2); return; /* (never: EDOP is seven bits) */
    case IS_MINUS_ZERO:
        incr(ADDRESS_OF(LOC));
        continue_at(NEWOPS);
        return; /* (never) */
    }
    switch (ccs_at(ADDRESS_OF(NEWJOB))) {
    case POSITIVE: continue_at(CHANG2); return;
    case IS_PLUS_ZERO: incr(ADDRESS_OF(LOC)); break;
    /* (never: NEWJOB is +0 or a positive core-set offset) */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: continue_at(NEWOPS + 1u); return;
    }
    continue_at(NEWOPS);
}

/* NEWOPS -- LM p. 1003, CM p. 1108.
 * "PICK UP OP CODE PAIR AND BEGIN." The word at LOC "MAY BE AN OPCODE
 * PAIR OR A STORE CODE": CCS A takes a positive word to DOSTORE and turns a
 * negative one into its two codes, the right one to EDOP (which keeps bits
 * 8-14 of what TS writes there) and the left one to OPJUMP. */
void newops(void)
{
    ca(indexed(0u, LOC));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: dostore(); return;
    case IS_PLUS_ZERO:
        /* (never: a program word is never +0. The AGC would run LOW7, OCT
         * 177, as TC 177.) */
        tc_at(ADDRESS_OF(LOW7));
        continue_at(LOW7);
        return;
    case NEGATIVE: ts(ADDRESS_OF(EDOP)); break;
    case IS_MINUS_ZERO: break; /* (never: no pair is -0) */
    }
    mask(LOW7);
    opjump();
}

/* OPJUMP -- LM p. 1003, CM p. 1108.
 * A holds a seven-bit order code (from NEWOPS, or from EDOP through
 * NOIBNKSW's CCS). TS CYR stores it rotated right one place, so CCS CYR
 * (which rotates it again) sees the code's bit 1 as its sign: negative for
 * an order with an address. A positive one goes on to test bit 2; +0 is the
 * code 0, EXIT. */
static void opjump(void)
{
    ts(CYR_ADDRESS);
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: opjump2(); return;
    case IS_PLUS_ZERO: exit_(); return;
    default: break; /* (CYR cannot be -0: the code has seven bits) */
    }
    /* ADDRESS -- LM p. 1004, CM p. 1109. "SEE IF ADDRESS IS INDEXED. CYR
     * CONTAINED 400XX, SO BIT 1 IS NOW AS IT WAS IN CYR." */
    mask(BIT1);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        index_(); /* A is 1 or +0 */
    else
        diradres();
}

/* DIRADRES -- LM p. 1004, CM p. 1109.
 * A direct address. "LOOK AHEAD TO NEXT WORD TO SEE IF ADDRESS IS GIVEN":
 * an address word is positive, an order pair negative. With no address the
 * operand comes from the push-down list (PUSHUP). DODLOAD and DOVLOAD (the
 * second address of STODL and STOVL) enter here too.
 *
 * Then "FINAL DIGESTION OF DIRECT ADDRESSES" (p. 1005): "ADDRESSES LESS
 * THAN 45D ARE TAKEN TO BE RELATIVE TO THE WORK AREA" (the job's VAC area,
 * at FIXLOC); addresses up to 3777 are general erasable, switched in through
 * EBANK; anything higher is in fixed memory, switched in through FBANK. */
void diradres(void)
{
    cs(indexed(1u, LOC));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: pushup(); return;
    case IS_PLUS_ZERO:
        /* (never: the next word is never -0. The AGC would run NEG4, DEC -4,
         * as MASK 7773, which leaves A +0 and goes on below.) */
        /* fall through */
    case NEGATIVE: incr(ADDRESS_OF(LOC)); break; /* "TO SHOW WE PICKED UP A WORD" */
    case IS_MINUS_ZERO: break;                   /* (never: an address word +0) */
    }
    ts(ADDRESS_OF(ADDRWD));
    ad(MINUS_ENDVAC);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* not in the work area */
        ad(MINUS_ENDERAS);
        /* fall through */
    case IS_PLUS_ZERO: ierastst(); return;
    case NEGATIVE: /* NETZERO: in the work area */
        ca(ADDRESS_OF(FIXLOC));
        /* fall through */
    case IS_MINUS_ZERO: /* address 45: the add leaves -0 */
        ads(ADDRESS_OF(ADDRWD));
        itr(ITR_7_INDJUMP); /* ITR15 */
        return;
    }
}

/* IERASTST -- LM p. 1005, CM p. 1110. General erasable or fixed. */
static void ierastst(void)
{
    if (A_is_zero_or_minus()) {
        /* GEADDR: switch EBANK and leave the address within it. */
        mask(LOW8);
        ad(OCT1400);
        xch(ADDRESS_OF(ADDRWD));
        ts(EBANK_ADDRESS);
        itr(ITR_7_INDJUMP); /* ITR10 */
        return;
    }
    /* "FIXED BANK ADDRESS. RESTORE AND ADD B15. SWITCH BANKS AND LEAVE
     * SUBADDRESS IN ADDRWD FOR OPERAND RETRIEVAL. (THIS METHOD PRECLUDES
     * USE OF THE LAST LOCATION IN EACH FBANK.)" */
    mask(LOW10);
    ad(LOW10);
    xch(ADDRESS_OF(ADDRWD));
    ad_at(ADDRESS_OF(INTBIT15));
    ts(FBANK_ADDRESS);
    itr(ITR_7_INDJUMP); /* ITR12 */
}

/* ITRnn: INDEX CYR; 7 INDJUMP -1 (or 3 INDJUMP -1). "THIS INDEX MAKES THE
 * NEXT INSTRUCTION TCF INDJUMP + OP, EDITING CYR": the instruction word
 * (code 7, or 3 for the indexed forms, and the address INDJUMP -1) plus
 * C(CYR), 20000 + the order's number (+ 40000 when indexed), carries out of
 * the code bits and comes back round as TCF INDJUMP + number. */
static void itr(word instruction)
{
    word made = indexed(instruction, fetch(CYR_ADDRESS));
    unsigned to = made & 07777u;

    if (to - INDJUMP < 040u)
        indjump(to - INDJUMP);
    else
        continue_at((word)to); /* (never: CYR always holds a code here) */
}

/* DODLOAD* -- LM p. 1006, CM p. 1111.
 * "STODL* COMES HERE TO PROCESS LOAD ADR." (DOVLOAD* enters at +1 with its
 * own code in A.) */
void dodload_star(void)
{
    set_A(DLOAD_STAR);
    dodload_star_plus1();
}

void dodload_star_plus1(void)
{
    ts(CYR_ADDRESS);
    index_();
}

/* INDEX -- LM p. 1006, CM p. 1111.
 * An indexed address. "AN INTERPRETER INDEX REGISTER MAY CONTAIN THE
 * ADDRESS OF ANY ERASABLE REGISTER (0-42 BEING RELATIVE TO THE VAC AREA) OR
 * ANY INTERPRETIVE PROGRAM BANK, OR ANY INTEGER IN THAT RANGE." The index
 * registers X1 and X2 are in the work area; INDEXLOC says which (FIXLOC
 * for X1, FIXLOC +1 for X2). The address word says X2 by being stored
 * complemented, and the register is subtracted from it. */
static void index_(void)
{
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(INDEXLOC));
    incr(ADDRESS_OF(LOC)); /* "(ADDRESS ALWAYS GIVEN)" */
    cs(indexed(0u, LOC));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        incr(ADDRESS_OF(INDEXLOC)); /* "INDEX 2 IF ADDRESS STORED COMPLEMENTED" */
        /* fall through */
    case IS_PLUS_ZERO: /* (NOOP) */
    case NEGATIVE:
        ts(ADDRESS_OF(ADDRWD)); /* "14 BIT ADDRESS TO ADDRWD" */
        break;
    case IS_MINUS_ZERO: break; /* (never: an address word +0) */
    }
    mask(HIGH4); /* "IF ADDRESS GREATER THAN 2K, ADD INTBIT15" */
    if (!A_is_zero()) {
        ca(ADDRESS_OF(INTBIT15));
        ads(ADDRESS_OF(ADDRWD));
    }
    index2();
}

/* INDEX2 -- LM p. 1006, CM p. 1111.
 * "DO AUGMENT, IGNORING AND CORRECTING OVF", then the address digested as
 * DIRADRES digests a direct one: work area, erasable bank or fixed bank. */
static void index2(void)
{
    cs(indexed(ADDRESS_OF(X1), INDEXLOC));
    ads(ADDRESS_OF(ADDRWD));
    mask(HIGH9); /* "SEE IF ADDRESS IS IN WORK AREA" */
    if (A_is_zero()) {
        ca(ADDRESS_OF(FIXLOC)); /* INDWORK */
        ads(ADDRESS_OF(ADDRWD));
        itr(ITR_3_INDJUMP); /* ITR13 */
        return;
    }
    mask(HIGH4); /* "SEE IF IN FIXED BANK" */
    if (A_is_zero()) {
        set_A(OCT1400); /* INDERASE */
        xch(ADDRESS_OF(ADDRWD));
        ts(EBANK_ADDRESS);
        mask(LOW8);
        ads(ADDRESS_OF(ADDRWD));
        itr(ITR_3_INDJUMP); /* ITR13 */
        return;
    }
    /* "IN FIXED -- SWITCH BANKS AND CREATE SUB-ADDRESS" */
    ca(ADDRESS_OF(ADDRWD));
    ts(FBANK_ADDRESS);
    mask(LOW10);
    ad(TWO_K);
    ts(ADDRESS_OF(ADDRWD));
    itr(ITR_3_INDJUMP); /* ITR11 */
}

/* PUSHUP -- LM p. 1008, CM p. 1113.
 * "WHEN NO OPERAND ADDRESS IS GIVEN, THE APPROPRIATE OPERAND IS TAKEN FROM
 * THE PUSH-DOWN LIST": PUSHLOC moves back by the operand's size (NO.WDS:
 * 6 words for a vector, 2 double precision, 3 triple) and ADDRWD gets it.
 * Mostly the size is that of MPAC's present mode. The exceptions are the
 * orders 00-03 (by CYR's bits 1, 2 and 5): VLOAD, TAD, SIGN and VXSC and
 * their neighbours want a definite size, or (VXSC, V/SC, code 3 mod 4)
 * "THE OPPOSITE TYPE OF OPERAND". */
static void pushup(void)
{
    set_A(OCT23);
    mask_at(CYR_ADDRESS);
    ad(MINUS_OCT10); /* "(NO -0)" */
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        /* REGUP: "MOST ALL OP CODES PUSHUP HERE." */
        cs(indexed(ADDRESS_OF(NO_WDS), MODE));
        regup_plus2();
        return;
    }
    /* "WE NOW HAVE 7 -- OP CODE (MOD4)": the masked code was 0-3. */
    ad(NEG4);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        cs(indexed(ADDRESS_OF(NO_WDS), a_word()));
    else
        cs(indexed(ADDRESS_OF(REVCNT), MODE)); /* -0: code 3, "REVERSE" */
    regup_plus2();
}

/* REGUP +2 -- LM p. 1008, CM p. 1113. A is minus the operand's size. */
static void regup_plus2(void)
{
    ads(ADDRESS_OF(PUSHLOC));
    ts(ADDRESS_OF(ADDRWD));
    itr(ITR_7_INDJUMP); /* ITR14: "(THE INDEX MAKES THIS A TCF.)" */
}

/* OPJUMP2 -- LM p. 1009, CM p. 1114.
 * "TEST SECOND PREFIX BIT TO SEE IF THIS IS A MISCELLANEOUS OR A
 * UNARY/SHORT SHIFT OPERATION." (CYR is +0 only for EXIT, taken before.) */
static void opjump2(void)
{
    if (ccs_at(CYR_ADDRESS) == POSITIVE)
        opjump3();
    else
        fifteen_bitadr();
}

/* 15BITADR -- LM p. 1009, CM p. 1114.
 * The miscellaneous orders take the next word whole: "WORK AREA ADDRESSES
 * IN THE RANGE 0-52, ERASABLE ECADR CONSTANTS FROM 100-3777, AND FCADRS
 * ABOVE THAT. ALL 15 BITS ARE AVAILABLE." It goes to POLISH, and the order
 * runs from MISCJUMP, in bank 1 ("THESE INSTRUCTIONS ARE IN BANK 1"). STCALL
 * enters here with CALL's code in CYR. */
void fifteen_bitadr(void)
{
    incr(ADDRESS_OF(LOC));
    ca(indexed(0u, LOC));
    ts(ADDRESS_OF(POLISH)); /* "WE MAY NEED A SUBADDRESS LATER." */
    set_A(LOW7_PLUS_2K);
    ts(FBANK_ADDRESS);
    mask_at(CYR_ADDRESS);
    miscjump(a_word()); /* ITR7: INDEX A; TCF MISCJUMP */
}

/* OPJUMP3 -- LM p. 1010, CM p. 1115.
 * "COMPLETE THE DISPATCHING OF UNARY AND SHORT SHIFT OPERATIONS": both are
 * in bank 0 ("BITS 11-15 OF A ARE 0"). The third prefix bit, now CYR's
 * sign, picks a unary order, by "THE DECREMENTED UNARY CODE" in A, or a
 * short shift, for a scalar (SHORTT) or a vector (SHORTV) by MODE. */
static void opjump3(void)
{
    ts(FBANK_ADDRESS);
    if (ccs_at(CYR_ADDRESS) == POSITIVE) {
        unajump(a_word()); /* INDEX A; TCF UNAJUMP */
        return;
    }
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: shortt(); return;
    case NEGATIVE: shortv(); return;
    case IS_MINUS_ZERO:
        /* (never: MODE is never -0. The AGC would run LVBUF, ADRES VBUF, as
         * TC VBUF.) */
        tc_at(ADDRESS_OF(LVBUF));
        continue_at(LVBUF);
        return;
    }
}

/* ======================================================================
 * The jump tables
 * ====================================================================== */

/* INDJUMP -- LM p. 1011, CM p. 1116.
 * "THE FOLLOWING IS THE JUMP TABLE FOR OP CODES WHICH MAY HAVE INDEXABLE
 * ADDRESSES OR MAY PUSH UP." ADDRWD holds the operand's address. */
void indjump(unsigned order)
{
    switch (order) {
    case 000: vload(); return;  /* load MPAC with a vector */
    case 001: tad(); return;    /* triple precision add */
    case 002: sign(); return;   /* complement MPAC if X negative */
    case 003: vxsc(); return;   /* vector times scalar */
    case 004: cgoto(); return;  /* computed go to */
    case 005: tload(); return;  /* load triple precision */
    case 006: dload(); return;  /* load a DP scalar */
    case 007: v_sc(); return;   /* vector divided by a scalar */
    case 010: sload(); return;  /* load single precision */
    case 011: ssp(); return;    /* set single precision into X */
    case 012: pddl(); return;   /* push down and DP load */
    case 013: mxv(); return;    /* matrix times vector */
    case 014: pdvl(); return;   /* push down and vector load */
    case 015: ccall(); return;  /* computed call */
    case 016: vxm(); return;    /* vector times matrix */
    case 017: tslc(); return;   /* normalize MPAC */
    case 020: dmpr(); return;   /* DP multiply and round */
    case 021: ddv(); return;    /* DP divide by */
    case 022: bddv(); return;   /* DP divide into */
    case 023: gshift(); return; /* general shift */
    case 024: vad(); return;    /* vector add */
    case 025: vsu(); return;    /* vector subtract */
    case 026: bvsu(); return;   /* vector subtract from */
    case 027: dot(); return;    /* dot product */
    case 030: vxv(); return;    /* cross product */
    case 031: vproj(); return;  /* vector projection */
    case 032: dsu(); return;    /* DP subtract */
    case 033: bdsu(); return;   /* DP subtract from */
    case 034: dad(); return;    /* DP add */
    case 035:                   /* "AVAILABLE": TCF +0, a TCF to itself (no program uses it) */
        continue_at(INDJUMP + 035u);
        return;
    case 036: dmp1(); return; /* DP multiply */
    default: setpd(); return; /* 037: set push down pointer */
    }
}

/* MISCJUMP -- LM p. 1012, CM p. 1117.
 * "THE FOLLOWING JUMP TABLE APPLIES TO INDEX, BRANCH, AND MISCELLANEOUS
 * INSTRUCTIONS." POLISH holds the address word. Where two orders share an
 * entry, CYR's sign (the code's bit 4) tells them apart. */
void miscjump(unsigned order)
{
    switch (order) {
    case 000: axt(); return;      /* address to index true */
    case 001: axc(); return;      /* address to index complemented */
    case 002: lxa(); return;      /* load index from erasable */
    case 003: lxc(); return;      /* load index from complement of erasable */
    case 004: sxa(); return;      /* store index in erasable */
    case 005: xchx(); return;     /* exchange index with erasable */
    case 006: incr_(); return;    /* increment index register */
    case 007: tix(); return;      /* transfer on index */
    case 010: xad(); return;      /* index register add from erasable */
    case 011: xsu(); return;      /* index subtract from erasable */
    case 012: bze_goto(); return; /* branch zero, and GOTO */
    case 013: bpl_bmn(); return;  /* branch plus, branch minus */
    case 014: rtb_bhiz(); return; /* return to basic, branch hi zero */
    case 015: call_ita(); return; /* call, and store QPRET */
    case 016: switches(); return; /* the switch orders (SW/) */
    default: bov_b(); return;     /* 017: branch on overflow */
    }
}

/* UNAJUMP -- LM p. 1013, CM p. 1118.
 * "THE FOLLOWING JUMP TABLE APPLIES TO UNARY INSTRUCTIONS", by the code
 * less one (00, EXIT, was "DETECTED EARLIER"). The table is in bank 0. */
void unajump(unsigned order)
{
    switch (order) {
    case 000: sqrt_(); return;    /* 01: square root */
    case 001: sine(); return;     /* 02: SIN */
    case 002: cosine(); return;   /* 03: COS */
    case 003: arcsin(); return;   /* 04: ARC SIN */
    case 004: arccos(); return;   /* 05: ARC COS */
    case 005: dsq(); return;      /* 06: DP square */
    case 006: round_(); return;   /* 07: round to DP */
    case 007: comp(); return;     /* 10: complement vector or scalar */
    case 010: vdef(); return;     /* 11: vector define */
    case 011: unit(); return;     /* 12: unit vector */
    case 012: abvalabs(); return; /* 13: length of vector or magnitude of scalar */
    case 013: vsq(); return;      /* 14: square of length of vector */
    case 014: stadr(); return;    /* 15: push up on store code */
    case 015: rvq(); return;      /* 16: return via QPRET */
    default: push(); return;      /* 17: push MPAC down */
    }
}
