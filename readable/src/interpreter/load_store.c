/* SPDX-License-Identifier: MIT */
/* The load and store package: INTERPRETER.agc section 2 (LM pp. 1014-1022,
 * CM pp. 1119-1127). The faithful C is c64-port/src/lm_interp_core.c
 * (dostore, store, lmi_dload_op, lmi_vload, lmi_pdvl, lmi_pddl_op,
 * lmi_push, lmi_ssp).
 *
 * "A SET OF EIGHT STORE CODES IS PROVIDED AS THE PRIMARY METHOD OF STORING
 * THE MULTI-PURPOSE ACCUMULATOR (MPAC). IF IN THE DANZIG SECTION LOC
 * REFERS TO AN ALGEBRAICALLY POSITIVE WORD, IT IS TAKEN AS A STORE CODE
 * WITH A CORRESPONDING ERASABLE ADDRESS."
 *	1. STORE	store MPAC; the address may be indexed.
 *	2. STODL	store and reload MPAC in DP from the next address.
 *	3. STOVL	store and reload a vector.
 *	4. STCALL	store and CALL. */
#include "interpreter.h"
#include "../agc/names.h"

static void storjump(word entry);
static void store_1(void);
static void store_2(void);
static void prestore(void);
static void startsto(void);
static void vstore_plus2(void);
static void endvload(void);
static void endvload_plus4(void);
static void endvpush(void);
static void enddpush(void);
static void endtpush(void);
static void vpdvl_plus4(void);

/* ======================================================================
 * Store codes
 * ====================================================================== */

/* STADR -- LM p. 1014, CM p. 1119.
 * The unary order STADR: the store code that follows it gets its address
 * from the push-down list. "THE STORECODE WAS STORED COMPLEMENTED TO MAKE
 * IT LOOK LIKE AN OPCODE PAIR. (YUL CAN'T REMOVE 1 BECAUSE OF EARLY CCS)" */
void stadr(void)
{
    ca(ADDRESS_OF(BANKSET));
    ts(FBANK_ADDRESS);
    incr(ADDRESS_OF(LOC));
    cs(indexed(0u, LOC)); /* ITR1 */
    ad(NEGONE);
    dostore();
}

/* DOSTORE -- LM p. 1014, CM p. 1119.
 * A store code, less one, in A: its low eleven bits the erasable address,
 * bits 12-14 which of the eight codes. "SAVE THE ERASABLE ADDRESS AND JUMP
 * ON THE STORE CODE NO." "EACH TRANSFER VECTOR ENTRY IS TWO WORDS." */
void dostore(void)
{
    ts(ADDRESS_OF(ADDRWD));
    mask(LOW11);
    xch(ADDRESS_OF(ADDRWD));
    mask(B12T14);
    mp(BIT5);
    storjump(a_word()); /* INDEX A; TCF STORJUMP */
}

/* STORJUMP -- LM p. 1015, CM p. 1120.
 * "STORE CODE JUMP TABLE. CALLS THE APPROPRIATE STORING ROUTINE AND EXITS
 * TO DANZIG OR TO ADDRESS WITH A SUPPLIED OPERATION CODE." Each entry is
 * a TC to the storing routine and the way on. "STORE STORE,1 AND STORE,2
 * RETURN TO DANZIG, THUS RESETTING THE EBANK TO ITS STATE AT INTPRET."
 * The second addresses of STODL and STOVL start a new pass of the
 * dispatcher at DODLOAD, DOVLOAD and the like. */
static void storjump(word entry)
{
    tc_at(indexed(STORJUMP, entry));
    switch (entry) {
    case 002: store_1(); break;
    case 004: store_2(); break;
    default: store_(); break;
    }
    switch (entry) {
    case 000:                                    /* STORE */
    case 002:                                    /* STORE,1 */
    case 004: continue_at(DANZIG); return;       /* STORE,2 */
    case 006: continue_at(DODLOAD); return;      /* STODL */
    case 010: continue_at(DODLOAD_STAR); return; /* STODL, indexed load address */
    case 012: continue_at(DOVLOAD); return;      /* STOVL */
    case 014: continue_at(DOVLOAD_STAR); return; /* STOVL, indexed load address */
    default:                                     /* 016: STCALL ("STOTC") */
        set_A(CALLCODE);
        ts(CYR_ADDRESS);
        continue_at(FIFTEEN_BITADR); /* "GET A 15 BIT ADDRESS." */
        return;
    }
}

/* STORE,1 and STORE,2 -- LM p. 1016, CM p. 1121.
 * The indexed stores: the index register subtracted from the address. */
static void store_1(void)
{
    cs(indexed(ADDRESS_OF(X1), FIXLOC));
    prestore();
}

static void store_2(void)
{
    cs(indexed(ADDRESS_OF(X2), FIXLOC));
    prestore();
}

/* PRESTORE: "RESULTANT ADDRESS IS IN ERASABLE." */
static void prestore(void)
{
    ads(ADDRESS_OF(ADDRWD));
    store_();
}

/* STORE -- LM p. 1016, CM p. 1121. (store() is the machine's store.)
 * "STORE CODE ADDRESS PROCESSOR": the address made absolute, in the work
 * area below 45 or through EBANK, then MPAC stored there. */
void store_(void)
{
    cs(ADDRESS_OF(ADDRWD));
    ad(DEC45);
    switch (ccs_at(A_ADDRESS)) {                  /* "DOES THE ADDRESS POINT TO THE WORK AREA?" */
    case POSITIVE: ca(ADDRESS_OF(FIXLOC)); break; /* "YES." */
    case IS_PLUS_ZERO: break;                     /* (never: no address gives +0) */
    case NEGATIVE:
        set_A(OCT1400); /* "NO. SET EBANK & MAKE UP SUBADDRESS." */
        /* fall through */
    case IS_MINUS_ZERO: /* address 45 */
        xch(ADDRESS_OF(ADDRWD));
        ts(EBANK_ADDRESS);
        mask(LOW8);
        break;
    }
    ads(ADDRESS_OF(ADDRWD)); /* AHEAD5 */
    startsto();
}

/* STARTSTO -- LM p. 1017, CM p. 1122.
 * "STORE DP, TP, OR VECTOR AS INDICATED BY MODE." "MPAC,+1 MUST BE STORED
 * IN ANY EVENT." */
static void startsto(void)
{
    dca(ADDRESS_OF(MPAC));
    dxch(indexed(0u, ADDRWD));
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE: /* TSTORE */
        ca(ADDRESS_OF(MPAC) + 2u);
        ts(indexed(2u, ADDRWD));
        tc_through(Q_ADDRESS);
        return;
    case IS_PLUS_ZERO: tc_through(Q_ADDRESS); return;
    case NEGATIVE: dca(ADDRESS_OF(MPAC) + 3u); break; /* VSTORE */
    case IS_MINUS_ZERO:
        /* (never: MODE is never -0. VSTORE's DCA would run without its
         * EXTEND, as CA MPAC +4.) */
        ca(ADDRESS_OF(MPAC) + 4u);
        break;
    }
    vstore_plus2();
}

static void vstore_plus2(void)
{
    dxch(indexed(2u, ADDRWD));
    dca(ADDRESS_OF(MPAC) + 5u);
    dxch(indexed(4u, ADDRWD));
    tc_through(Q_ADDRESS);
}

/* ======================================================================
 * The second address of STODL and STOVL
 * ====================================================================== */

/* DODLOAD, DOVLOAD -- LM p. 1018, CM p. 1123.
 * "ROUTINES TO BEGIN PROCESSING OF THE SECOND ADDRES ASSOCIATED WITH ALL
 * STORE-TYPE CODES EXCEPT STORE ITSELF": CYR gets the load's order code, as
 * OPJUMP would leave it, and DIRADRES (the next pass) "GET[S] A DIRECT
 * ADDRESS". */
void dodload(void)
{
    set_A(DLOADCOD);
    ts(CYR_ADDRESS);
    continue_at(DIRADRES);
}

void dovload(void)
{
    set_A(VLOADCOD);
    ts(CYR_ADDRESS);
    continue_at(DIRADRES);
}

/* DOVLOAD* -- LM p. 1018, CM p. 1123. "PROLOGUE TO INDEX ROUTINE." */
void dovload_star(void)
{
    set_A(VLOAD_STAR);
    dodload_star_plus1();
}

/* ======================================================================
 * Loads
 * ====================================================================== */

/* TLOAD -- LM p. 1019, CM p. 1124.
 * "LOAD A TRIPLE PRECISION ARGUMENT INTO THE FIRST THREE MPAC REGISTERS,
 * WITH THE CONTENTS OF THE OTHER FOUR IRRELEVANT." */
void tload(void)
{
    ca(indexed(2u, ADDRWD));
    ts(ADDRESS_OF(MPAC) + 2u);
    dca(indexed(0u, ADDRWD));
    tload_plus6();
}

void tload_plus6(void)
{
    dxch(ADDRESS_OF(MPAC));
    tmode();
}

/* TMODE: "DECLARE TRIPLE PRECISION MODE." */
void tmode(void)
{
    set_A(ONE);
    newmode();
}

/* SLOAD -- LM p. 1019, CM p. 1124.
 * "LOAD A SINGLE PRECISION NUMBER INTO MPAC, SETTING MPAC+1,2 TO ZERO." */
void sload(void)
{
    zl();
    ca(indexed(0u, ADDRWD));
    sload2();
}

/* VLOAD -- LM p. 1019, CM p. 1124.
 * "LOAD A DOUBLE PRECISION VECTOR INTO MPAC,+1, MPAC+3,4, AND MPAC+5,6."
 * ENDVLOAD and ENDVLOAD +4 are where PDVL finishes. */
void vload(void)
{
    dca(indexed(0u, ADDRWD));
    dxch(ADDRESS_OF(MPAC));
    endvload();
}

static void endvload(void)
{
    dca(indexed(2u, ADDRWD));
    dxch(ADDRESS_OF(MPAC) + 3u);
    endvload_plus4();
}

static void endvload_plus4(void)
{
    dca(indexed(4u, ADDRWD));
    dxch(ADDRESS_OF(MPAC) + 5u);
    vmode();
}

/* VMODE: "DECLARE VECTOR MODE." */
void vmode(void)
{
    set_A(negate(ONE));
    newmode();
}

/* ======================================================================
 * The push-down list
 * ====================================================================== */

/* PDDL -- LM p. 1020, CM p. 1125.
 * Push MPAC down and load a DP scalar: "LOAD MPAC,+1, PUSHING THE FORMER
 * CONTENTS DOWN", then "ADVANCE THE PUSHDOWN POINTER APPROPRIATELY" and
 * push the rest by the old mode. */
void pddl(void)
{
    dca(indexed(0u, ADDRWD));
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed(0u, PUSHLOC));
    ca(indexed(ADDRESS_OF(NO_WDS), MODE));
    ads(ADDRESS_OF(PUSHLOC));
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE: endtpush(); return;
    case IS_PLUS_ZERO: enddpush(); return;
    case NEGATIVE: ts(ADDRESS_OF(MODE)); break; /* "NOW DP." */
    case IS_MINUS_ZERO: break;                  /* (never: MODE -0) */
    }
    endvpush();
}

/* ENDVPUSH: "PUSH DOWN THE REST OF THE VECTOR HERE." */
static void endvpush(void)
{
    ts(ADDRESS_OF(MPAC) + 2u);
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(PUSHED(4u));
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(PUSHED(2u));
    continue_at(DANZIG);
}

/* ENDDPUSH: "SET MPAC +2 TO ZERO AND EXIT ON DP." */
static void enddpush(void)
{
    ts(ADDRESS_OF(MPAC) + 2u);
    continue_at(DANZIG);
}

/* ENDTPUSH: "ON TRIPLE, SET MPAC +2 TO ZERO, PUSHING DOWN THE OLD
 * CONTENTS". */
static void endtpush(void)
{
    ts(ADDRESS_OF(MODE));
    xch(ADDRESS_OF(MPAC) + 2u);
    endtpush_plus2();
}

void endtpush_plus2(void)
{
    ts(PUSHED(1u));
    continue_at(DANZIG);
}

/* PDVL -- LM p. 1021, CM p. 1126.
 * "PUSHDOWN AND VECTOR LOAD": "RELOAD MPAC AND PUSH DOWN ITS CONTENTS." On
 * a triple precision MPAC, "WE MUST LOAD THE Y COMPONENT BEFORE STORING MPAC
 * +2 IN CASE THIS IS A PUSHUP" (the load's address may be the word being
 * pushed). */
void pdvl(void)
{
    dca(indexed(0u, ADDRWD));
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed(0u, PUSHLOC));
    ca(indexed(ADDRESS_OF(NO_WDS), MODE));
    ads(ADDRESS_OF(PUSHLOC));
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE: /* TPDVL */
        dca(indexed(2u, ADDRWD));
        dxch(ADDRESS_OF(MPAC) + 3u);
        ca(ADDRESS_OF(MPAC) + 2u);
        ts(PUSHED(1u));
        endvload_plus4();
        return;
    case IS_PLUS_ZERO: endvload(); return; /* "JUST LOAD LAST FOUR REGISTERS ON DP." */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: /* (never: MODE -0. VPDVL's DCA would run as CA.) */
        ca(indexed(3u, ADDRWD));
        dxch(ADDRESS_OF(MPAC) + 3u);
        vpdvl_plus4();
        return;
    }
    /* VPDVL: "PUSHDOWN AND RE-LOAD LAST TWO COMPONENTS" */
    dca(indexed(2u, ADDRWD));
    dxch(ADDRESS_OF(MPAC) + 3u);
    vpdvl_plus4();
}

/* VPDVL +4: push the old Y and Z down, load Z. */
static void vpdvl_plus4(void)
{
    dxch(PUSHED(4u));
    dca(indexed(4u, ADDRWD));
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(PUSHED(2u));
    continue_at(DANZIG);
}

/* SSP -- LM p. 1021, CM p. 1126.
 * "SSP (STORE SINGLE PRECISION) IS EXECUTED HERE": "PICK UP THE WORD
 * FOLLOWING THE GIVEN ADDRESS AND STORE IT AT X." */
void ssp(void)
{
    incr(ADDRESS_OF(LOC));
    ca(indexed(0u, LOC));
    store1();
}

/* STORE1: "SOME INDEX AND MISCELLANEOUS OPS END HERE." */
void store1(void)
{
    ts(indexed(0u, ADDRWD));
    continue_at(DANZIG);
}
