/* SPDX-License-Identifier: MIT */
/* The index register orders and the miscellaneous orders with direct
 * addresses: INTERPRETER.agc LM pp. 1087-1091, CM pp. 1192-1196 (in bank
 * 1). The faithful C is c64-port/src/lm_interp_core.c (lmi_axt to lmi_tix,
 * adrers15, tagsub, lmi_rtb, lmi_bhiz, lmi_bov, lmi_bzegoto, lmi_bplbmn,
 * lmi_call, lmi_ita).
 *
 * "THE FOLLOWING INSTRUCTIONS ARE AVAILABLE FOR SETTING, MODIFYING, AND
 * BRANCHING ON INDEX REGISTERS:
 *	1. AXT	ADDRESS TO INDEX TRUE.
 *	2. AXC	ADDRESS TO INDEX COMPLEMENTED.
 *	3. LXA	LOAD INDEX FROM ERASABLE.
 *	4. LXC	LOAD INDEX COMPLEMENTED FROM ERASABLE.
 *	5. SXA	STORE INDEX IN ERASABLE.
 *	6. XCHX	EXCHANGE INDEX REGISTER WITH ERASABLE.
 *	7. INCR	INCREMENT INDEX REGISTER.
 *	8. XAD	ERASABLE ADD TO INDEX REGISTER.
 *	9. XSU	ERASABLE SUBTRACT FROM INDEX REGISTER.
 *	10. TIX	BRANCH ON INDEX REGISTER AND DECREMENT."
 *
 * Each job's work area (at FIXLOC) holds its two index registers, X1 and
 * X2, and their step sizes S1 and S2. These orders take their whole
 * address word from POLISH (15BITADR). */
#include "interpreter.h"
#include "../agc/names.h"
#include "../interbank/interbank.h"

static void xstore(void);
static void mstore1(void);
static void xad2(void);
static void dotixbr(void);
static void fifteen_adrers_plus9(void);
static void rtb_plus1(void);
static void bhiz(void);
static void bpl(void);

/* The index register (X1, or X2 one word on) or step (S1) that TAGSUB
 * chose: INDEX INDEXLOC on the work area's X1 or S1. */
#define X_SELECTED (indexed(ADDRESS_OF(X1), INDEXLOC))
#define S_SELECTED (indexed(ADDRESS_OF(S1), INDEXLOC))

/* ======================================================================
 * The index register orders
 * ====================================================================== */

/* AXT, AXC -- LM p. 1087, CM p. 1192. The address word itself, or its
 * complement, into the index register. "SELECT APPROPRIATE INDEX
 * REGISTER." */
void axt(void)
{
    tc_at(AXT);
    tagsub();
    ca(ADDRESS_OF(POLISH));
    xstore();
}

/* XSTORE: INDEXLOC "CONTAINS C(FIXLOC) OR C(FIXLOC)+1". */
static void xstore(void)
{
    ts(X_SELECTED);
    continue_at(DANZIG);
}

void axc(void)
{
    tc_at(AXC);
    tagsub();
    cs(ADDRESS_OF(POLISH));
    tc_at(AXC + 2u); /* (a TC, not a TCF: Q is left at AXC +3) */
    xstore();
}

/* LXA, LXC -- LM p. 1087, CM p. 1192. "LOAD INDEX REGISTER FROM ERASABLE",
 * or "FROM ERASABLE COMPLEMENTED". */
void lxa(void)
{
    tc_at(LXA);
    fifteen_adrers();
    ca(indexed(0u, POLISH));
    xstore();
}

void lxc(void)
{
    tc_at(LXC);
    fifteen_adrers();
    cs(indexed(0u, POLISH));
    xstore();
}

/* SXA -- LM p. 1087, CM p. 1192. "STORE INDEX REGISTER IN ERASABLE." */
void sxa(void)
{
    tc_at(SXA);
    fifteen_adrers();
    ca(X_SELECTED);
    mstore1();
}

/* MSTORE1: A into the erasable word at POLISH. */
static void mstore1(void)
{
    ts(indexed(0u, POLISH));
    continue_at(DANZIG);
}

/* XCHX -- LM p. 1088, CM p. 1193. "EXCHANGE INDEX REGISTER WITH ERASABLE." */
void xchx(void)
{
    tc_at(XCHX);
    fifteen_adrers();
    ca(indexed(0u, POLISH));
    xch(X_SELECTED);
    mstore1();
}

/* XAD -- LM p. 1088, CM p. 1193. "ADD ERASABLE TO INDEX REGISTER." */
void xad(void)
{
    tc_at(XAD);
    fifteen_adrers();
    ca(indexed(0u, POLISH));
    xad2();
}

static void xad2(void)
{
    ads(X_SELECTED); /* "IGNORING OVERFLOWS." */
    continue_at(DANZIG);
}

/* INCR -- LM p. 1088, CM p. 1193. "INCREMENT INDEX REGISTER" by the
 * address word. */
void incr_(void)
{
    tc_at(INCR);
    tagsub();
    ca(ADDRESS_OF(POLISH));
    xad2();
}

/* XSU -- LM p. 1088, CM p. 1193. "SUBTRACT ERASABLE FROM INDEX REGISTER." */
void xsu(void)
{
    tc_at(XSU);
    fifteen_adrers();
    cs(indexed(0u, POLISH));
    xad2();
}

/* TIX -- LM p. 1088, CM p. 1193. "BRANCH AND DECREMENT ON INDEX": the index
 * less its step; "NO OPERATION IF DECREMENTED INDEX IS NEGATIVE OR ZERO",
 * otherwise the index takes it and "DO THE BRANCH USING THE CADR IN
 * POLISH." */
void tix(void)
{
    tc_at(TIX);
    tagsub();
    cs(S_SELECTED);
    ad_at(X_SELECTED);
    if (A_is_zero_or_minus()) {
        continue_at(DANZIG);
        return;
    }
    dotixbr();
}

static void dotixbr(void)
{
    xch(X_SELECTED); /* "IGNORING OVERFLOWS." */
    goto_();
}

/* 15ADRERS -- LM p. 1089, CM p. 1194. "SUBROUTINE TO CONVERT AN ERASABLE
 * ADDRESS (11 BITS) TO AN EBANK SETTING AND SUBADDRESS." "DOES THE ADDRESS
 * POINT TO THE WORK AREA?" "YES. ADD FIXLOC. EBANK OK AS IS." "NO. SET
 * EBANK & MAKE UP SUBADDRESS." Then "FALL INTO TAGSUB, AND RETURN VIA Q."
 * (An address of exactly 45 is left as it is.) */
void fifteen_adrers(void)
{
    cs(ADDRESS_OF(POLISH));
    ad(DEC45);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ca(ADDRESS_OF(FIXLOC)); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
        set_A(OCT1400);
        /* fall through */
    case IS_MINUS_ZERO:
        xch(ADDRESS_OF(POLISH));
        ts(EBANK_ADDRESS);
        mask(LOW8);
        break;
    }
    fifteen_adrers_plus9();
}

static void fifteen_adrers_plus9(void)
{
    ads(ADDRESS_OF(POLISH));
    tagsub();
}

/* TAGSUB -- LM p. 1089, CM p. 1194. "SUBROUTINE WHICH SETS THE ADDRESS OF
 * THE SPECIFIED INDEX IN INDEXLOC. (ACTUALLY, THE ADDRESS -38D.)" "BIT 15
 * SPECIFIES INDEX", which CYR's sign now shows: "0 MEANS USE X2", "1 FOR
 * X1". (A CYR of -0, which an order code never makes, would run on into
 * RTB/BHIZ.) */
void tagsub(void)
{
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(INDEXLOC));
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: incr(ADDRESS_OF(INDEXLOC)); /* fall through */
    case IS_PLUS_ZERO:
    case NEGATIVE: tc_through(Q_ADDRESS); return;
    case IS_MINUS_ZERO: rtb_bhiz(); return;
    }
}

/* ======================================================================
 * RTB, BHIZ, BOV, BZE, GOTO, BPL, BMN, CALL, ITA
 * ====================================================================== */

/* RTB/BHIZ -- LM p. 1090, CM p. 1195.
 * "MISCELLANEOUS OPERATION CODES WITH DIRECT ADDRESSES":
 *	"RTB	RETURN TO BASIC LANGUAGE AT THE GIVEN ADDRESS.
 *	 BHIZ	BRANCH IF THE HIGH ORDER OF MPAC IS ZERO (SINGLE PRECISION)."
 * One table entry for both: CYR's sign tells them apart. */
void rtb_bhiz(void)
{
    /* The slots: RTB, RTB +1 (+0), BHIZ, BHIZ +1 (-0). */
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: rtb(); return;
    case IS_PLUS_ZERO: rtb_plus1(); return;
    case NEGATIVE: bhiz(); return;
    case IS_MINUS_ZERO: bhiz_plus1(); return;
    }
}

/* RTB: SWCALL with the CADR in POLISH, "SO A "TC Q" FROM ROUTINE LEADS TO
 * DANZIG" (SWRETURN, then RTB_RETURN). TC SWCALL -1 increments Q past the
 * TC and runs on into SWCALL. */
void rtb(void)
{
    ca(ADDRESS_OF(POLISH));
    rtb_plus1();
}

static void rtb_plus1(void)
{
    tc_at(RTB + 1u);
    swcall_minus1();
    swcall();
}

/* BHIZ: "BRANCH IF THE HIGH ORDER OF MPAC IS ZERO". */
static void bhiz(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE:
    case NEGATIVE: continue_at(DANZIG); return;
    default: goto_(); return;
    }
}

/* BHIZ +1: the -0 slot of RTB/BHIZ's CCS. */
void bhiz_plus1(void)
{
    continue_at(DANZIG);
}

/* BOV(B) -- LM p. 1090, CM p. 1195. "BRANCH ON OVERFLOW TO BASIC OR
 * INTERP." BOV and BOVB: if OVFIND is set, clear it and branch, by GOTO
 * (BOV) or by RTB (BOVB, to basic code); CYR's sign tells which. (A CYR of
 * +0 would run the next word, B5TOBB, OCT 360, as TC 360; -0 would run on
 * into BZE/GOTO.) */
void bov_b(void)
{
    switch (ccs_at(ADDRESS_OF(OVFIND))) {
    case IS_PLUS_ZERO: continue_at(DANZIG); return;
    case IS_MINUS_ZERO: break;
    default: ts(ADDRESS_OF(OVFIND)); break;
    }
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: rtb(); return; /* "IF BASIC." */
    case IS_PLUS_ZERO:
        tc_at(ADDRESS_OF(B5TOBB));
        continue_at(B5TOBB);
        return;
    case NEGATIVE: goto_(); return;
    case IS_MINUS_ZERO: bze_goto(); return;
    }
}

/* BZE/GOTO -- LM p. 1091, CM p. 1196. "SEE WHICH OP-CODE IS DESIRED": BZE
 * ("DO BZE": TC BRANCH, whose three returns are the next three words) or
 * GOTO. */
void bze_goto(void)
{
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE:
        tc_at(BZE_GOTO + 1u);
        if (branch() == BRANCH_ZERO)
            goto_();
        else
            continue_at(DANZIG);
        return;
    case NEGATIVE: goto_(); return; /* "DO GOTO." */
    default: continue_at(DANZIG); return;
    }
}

/* BPL/BMN -- LM p. 1091, CM p. 1196. BPL branches on MPAC "POSITIVE OR
 * ZERO", BMN "ONLY IF NNZ" (negative non-zero). (A CYR of +0 would run the
 * next word, the constant 5B10, as TCF 2000.) */
void bpl_bmn(void)
{
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: bpl(); return;
    case IS_PLUS_ZERO: continue_at(FIVE_B10 & 07777u); return;
    case NEGATIVE: /* "DO BMN" */
        tc_at(BPL_BMN + 3u);
        if (branch() == BRANCH_NEGATIVE)
            goto_();
        else
            continue_at(DANZIG);
        return;
    case IS_MINUS_ZERO: continue_at(DANZIG); return;
    }
}

static void bpl(void)
{
    tc_at(BPL);
    if (branch() == BRANCH_NEGATIVE)
        continue_at(DANZIG);
    else
        goto_(); /* "IF POSITIVE OR ZERO." */
}

/* CALL/ITA -- LM p. 1091, CM p. 1196.
 *	"ITA	STORE CPRET (RETURN ADDRESS) IN ERASABLE.
 *	 CALL	CALL A SUBROUTINE, LEAVING RETURN IN QPRET."
 * "STORE QPRET. (TAGSUB AFTER 15ADRERS IS SLOW IN THIS CASE, BUT SAVES
 * STORAGE.)" (CYR is never +0 here: that slot is TC CCSHOLE. A -0 would
 * skip the TC 15ADRERS.) */
void call_ita(void)
{
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: call(); return;
    case IS_PLUS_ZERO:
        tc_at(CALL_ITA + 2u);
        continue_at(CCSHOLE);
        return;
    case NEGATIVE:
        tc_at(CALL_ITA + 3u);
        fifteen_adrers();
        break;
    case IS_MINUS_ZERO: break;
    }
    ca(indexed(ADDRESS_OF(QPRET), FIXLOC));
    mstore1();
}
