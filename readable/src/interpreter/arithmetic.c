/* SPDX-License-Identifier: MIT */
/* The add/subtract package and the arithmetic subroutines: INTERPRETER.agc
 * section 3, LM pp. 1026-1037, CM pp. 1131-1142. The faithful C is
 * c64-port/src/lm_interp_core.c (vaddsub, endvxv, overflow, dmpsub,
 * roundsub, dotsub, lmi_poly, tpagree) and, for the routines other files
 * call, the basic table c64-port/src/lm_interpreter.c.
 *
 * Numbers are fractions in one's complement, double precision in two words
 * (major part first) or triple in three. A double-precision add (DAS) leaves
 * the overflow out of the major word, +1, -1 or +0, in A: "THE INTERPRETIVE
 * OVERFLOW INDICATOR OVFIND IS SET NON-ZERO IF OVERFLOW OCCURS". */
#include "interpreter.h"
#include "../agc/names.h"

static void read_indexed(word channel_instruction);
static void overflwy_plus2(void);
static void polycom(void);
static void poly2(void);
static void dpagree_plus1(void);
static void argzero2(void);

/* ======================================================================
 * DAD, DSU, BDSU, VAD, VSU, BVSU, TAD
 * ====================================================================== */

/* VSU, VAD -- LM p. 1026, CM p. 1131.
 * Vector add and subtract. One body serves both: ADDRWD is made 30000 or
 * 40000 more, and each "EXTEND; INDEX ADDRWD; READ <channel>" then runs as
 * DCA or DCS of the operand's next component ("CHANGES 0 TO DCA"). The
 * components are added from the last: "DCA 2 OR DCS 2", 4, then 0. */
void vsu(void)
{
    set_A(BIT15); /* "CHANGES 0 TO DCS." */
    vad_plus1();
}

void vad(void)
{
    set_A(PRIO30); /* "CHANGES 0 TO DCA." */
    vad_plus1();
}

void vad_plus1(void)
{
    ads(ADDRESS_OF(ADDRWD));
    read_indexed(READ_HISCALAR);
    das(MPAC_AT(3u));
    if (!A_is_zero()) {
        tc_at(VAD + 8u);
        overflwy();
    }
    read_indexed(READ_CHAN5);
    das(MPAC_AT(5u));
    if (!A_is_zero()) {
        tc_at(VAD + 15u);
        overflwz();
    }
    read_indexed(READ_LCHAN);
    endvxv();
}

/* EXTEND; INDEX ADDRWD; READ k: the READ (code 0, the channel k) plus
 * ADDRWD is the extracode DCA or DCS (codes 3 and 4) of ADDRWD's low
 * address + k - 1 (DCA and DCS carry their address plus one). */
static void read_indexed(word channel_instruction)
{
    word made = indexed(channel_instruction, ADDRWD);
    unsigned k = (made & 07777u) - 1u;

    if ((made >> 12) == 4u)
        dcs(k);
    else
        dca(k);
}

/* DAD -- LM p. 1026, CM p. 1131. Double precision add. */
void dad(void)
{
    dca(indexed(0u, ADDRWD));
    endvxv();
}

/* ENDVXV -- LM p. 1026, CM p. 1131. "VXV FINISHES HERE." The DP sum into
 * MPAC, and OVFIND set if it overflowed (SETOVF). */
void endvxv(void)
{
    das(MPAC_AT(0u));
    if (A_is_zero()) {
        continue_at(DANZIG);
        return;
    }
    setovf();
}

/* SETOVF -- LM p. 1027, CM p. 1132. */
void setovf(void)
{
    tc_at(SETOVF);
    overflow();
    continue_at(DANZIG);
}

/* DSU -- LM p. 1028, CM p. 1133. Double precision subtract. */
void dsu(void)
{
    dcs(indexed(0u, ADDRWD));
    endvxv();
}

/* OVERFLWZ, OVERFLWY -- LM p. 1028, CM p. 1133.
 * OVERFLOW for the third and second components: L gets 5 or 3, the
 * offset of the component in MPAC, and A keeps the overflow. */
void overflwz(void)
{
    ts(L_ADDRESS); /* "ENTRY FOR THIRD COMPONENT." */
    set_A(FIVE);
    overflwy_plus2();
}

void overflwy(void)
{
    ts(L_ADDRESS); /* "ENTRY FOR SECOND COMPONENT." */
    set_A(THREE);
    overflwy_plus2();
}

static void overflwy_plus2(void)
{
    xch(L_ADDRESS);
    overflow();
}

/* OVERFLOW -- LM p. 1028, CM p. 1133.
 * "ENTRY FOR 1ST COMP OR DP (L=0)." A component of MPAC (at L) overflowed
 * by A (+1 or -1). The component is set to the largest number of the
 * overflow's sign: POSMAX or NEGMAX (from LIMITS) is added with an
 * overflow forced into it, which cancels the one there. TS 7 (the zero
 * register) only tests for overflow. If that still overflows, OVFIND is
 * set (SETOVF2); the return is the TC Q either way. */
void overflow(void)
{
    cs(indexed(ADDRESS_OF(LIMITS), a_word())); /* "PICK UP POSMAX OR NEGMAX." */
    ts(ADDRESS_OF(BUF));
    aug(A_ADDRESS); /* "FORCE OVERFLOW." */
    ads(indexed(MPAC_AT(1u), overflow_corrected(L)));
    if (!ts(ZERO_ADDRESS)) set_A(ZERO);
    ad_at(ADDRESS_OF(BUF));
    ads(indexed(MPAC_AT(0u), overflow_corrected(L)));
    if (ts(ZERO_ADDRESS))
        setovf2();
    else
        tc_through(Q_ADDRESS); /* "NO OVERFLOW EXIT." */
}

/* BVSU -- LM p. 1028, CM p. 1133. Vector subtract from: MPAC is the
 * operand less MPAC, from the last component (DCOM complements the
 * component in A,L). */
void bvsu(void)
{
    dca(indexed(2u, ADDRWD));
    dxch(MPAC_AT(3u));
    dcom();
    das(MPAC_AT(3u));
    if (!A_is_zero()) {
        tc_at(BVSU + 9u);
        overflwy();
    }
    dca(indexed(4u, ADDRWD));
    dxch(MPAC_AT(5u));
    dcom();
    das(MPAC_AT(5u));
    if (!A_is_zero()) {
        tc_at(BVSU + 19u);
        overflwz();
    }
    bdsu();
}

/* BDSU -- LM p. 1029, CM p. 1134. Double precision subtract from. */
void bdsu(void)
{
    dca(indexed(0u, ADDRWD));
    dxch(MPAC_AT(0u));
    dcom();
    endvxv();
}

/* TAD -- LM p. 1030, CM p. 1135.
 * "TRIPLE PRECISION ADD ROUTINE": "ADD MINOR PARTS FIRST", then the major
 * part with the carry, OVFIND set if it overflows. */
void tad(void)
{
    dca(indexed(1u, ADDRWD));
    das(MPAC_AT(1u));
    ad_at(indexed(0u, ADDRWD));
    ad_at(MPAC_AT(0u));
    if (ts(MPAC_AT(0u)))
        setovf(); /* "SET OVFIND IF SUCH OCCURS." */
    else
        continue_at(DANZIG);
}

/* ======================================================================
 * Multiplication and rounding
 * ====================================================================== */

/* DMP -- LM p. 1031, CM p. 1136.
 * "BASIC SUBROUTINE FOR USE BY PINBALL, ETC": DMPSUB with the operand's
 * address in the word after the TC DMP ("ADRES OF ARGUMENT FOLLOWS"). */
void dmp(void)
{
    ca(indexed(0u, overflow_corrected(Q)));
    incr(Q_ADDRESS);
    dmpsub_minus1();
}

/* DMPSUB -1: "(PROLOGUE FOR SETTING ADDRWD.)" */
void dmpsub_minus1(void)
{
    ts(ADDRESS_OF(ADDRWD));
    dmpsub();
}

/* DMPSUB -- LM p. 1031, CM p. 1136.
 * "DOUBLE PRECISION MULTIPLY, MULTIPLY THE CONTENTS OF MPAC,+1 BY THE DP
 * WORD WHOSE ADDRESS IS IN ADDRWD AND LEAVE A TRIPLE-PRECISION RESULT IN
 * MPAC." Four partial products, accumulated with DAS from the least
 * significant ("49 MCT = .573 MS. INCLUDING RETURN"). */
void dmpsub(void)
{
    ca(indexed(1u, ADDRWD)); /* "GET MINOR PART OF OPERAND AT C(ADDRWD)." */
    ts(MPAC_AT(2u));         /* "THIS WORKS FOR SQUARING MPAC AS WELL." */
    set_A(ZERO);
    xch(MPAC_AT(1u));
    ts(ADDRESS_OF(MPTEMP));
    mp_at(MPAC_AT(2u)); /* "MINOR OF MPAC X MINOR OF C(ADDRWD)." */
    xch(MPAC_AT(2u));   /* "DISCARD MINOR PART OF ABOVE RESULT" */
    mp_at(MPAC_AT(0u)); /* major of MPAC x minor of the operand */
    das(MPAC_AT(1u));   /* "GUARANTEED NO OVERFLOW." */
    ca(indexed(0u, ADDRWD));
    xch(ADDRESS_OF(MPTEMP));
    dmpsub2();
}

/* DMPSUB2: major of the operand (in MPTEMP) times what A holds, then times
 * the major of MPAC. */
void dmpsub2(void)
{
    mp_at(ADDRESS_OF(MPTEMP));
    das(MPAC_AT(1u)); /* "ACCUMULATE, SETTING A TO NET OVERFLOW." */
    xch(MPAC_AT(0u)); /* "SETTING MPAC TO 0 OR +-1." */
    mp_at(ADDRESS_OF(MPTEMP));
    das(MPAC_AT(0u)); /* "GUARANTEED NO OVERFLOW." */
    tc_through(Q_ADDRESS);
}

/* ROUNDSUB -- LM p. 1032, CM p. 1137.
 * "ROUND MPAC TO DOUBLE PRECISION, SETTING OVFIND ON THE RARE EVENT OF
 * OVERFLOW." "SET MPAC +2 = 0 FOR SCALARS AND CHANGE MODE TO DP." */
void roundsub(void)
{
    set_A(ZERO);
    roundsub_plus1();
}

void roundsub_plus1(void)
{
    ts(ADDRESS_OF(MODE));
    vround();
}

/* VROUND: "BUT WE NEEDN'T TAKE THE TIME FOR VECTORS." MPAC +2 doubled
 * overflows when it is .5 or more in magnitude, and the overflow is the
 * rounding bit, added on up the words. */
void vround(void)
{
    xch(MPAC_AT(2u));
    double_A();
    if (!ts(L_ADDRESS)) {
        tc_through(Q_ADDRESS);
        return;
    }
    ad_at(MPAC_AT(1u)); /* "ADD ROUDING BIT IF MPAC +2 WAS GREATER THAN .5" */
    if (!ts(MPAC_AT(1u))) {
        tc_through(Q_ADDRESS);
        return;
    }
    ad_at(MPAC_AT(0u)); /* "PROPAGATE INTERFLOW." */
    if (!ts(MPAC_AT(0u))) {
        tc_through(Q_ADDRESS);
        return;
    }
    setovf2();
}

/* SETOVF2: "(RARE)." */
void setovf2(void)
{
    ts(ADDRESS_OF(OVFIND));
    tc_through(Q_ADDRESS);
}

/* ======================================================================
 * The dot product
 * ====================================================================== */

/* PREDOT, DOTSUB -- LM p. 1033, CM p. 1138.
 * "THE DOT PRODUCT SUBROUTINE USUALLY FORMS THE DOT PRODUCT OF THE VECTOR
 * IN MPAC WITH A STANDARD SIX REGISTER VECTOR WHOSE ADDRESS IS IN ADDRWD.
 * IN THIS CASE C(DOTINC) ARE SET TO 2. VXM, HOWEVER, SETS C(DOTINC) TO 6
 * SO THAT DOTSUB DOTS MPAC WITH A COLUMN VECTOR OF THE MATRIX". The sum
 * builds in the three-word BUF. */
void predot(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(DOTINC));
    dotsub();
}

void dotsub(void)
{
    qxch(ADDRESS_OF(DOTRET)); /* "SAVE RETURN" */
    tc_at(DOTSUB + 2u);
    dmpsub(); /* "DOT X COMPONENTS." */
    dxch(MPAC_AT(3u));
    dxch(MPAC_AT(0u));
    dxch(ADDRESS_OF(BUF));
    ca(MPAC_AT(2u));
    ts(ADDRESS_OF(BUF) + 2u);

    ca(ADDRESS_OF(DOTINC)); /* "ADVANCE ADDRWD TO Y COMPONENT" */
    ads(ADDRESS_OF(ADDRWD));
    tc_at(DOTSUB + 10u);
    dmpsub();
    dxch(MPAC_AT(1u)); /* "ACCUMULATE PARTIAL PRODUCTS." */
    das(ADDRESS_OF(BUF) + 1u);
    ad_at(MPAC_AT(0u));
    ad_at(ADDRESS_OF(BUF));
    if (ts(ADDRESS_OF(BUF))) ts(ADDRESS_OF(OVFIND)); /* "IF OVERFLOW OCCURS." */

    dxch(MPAC_AT(5u)); /* "MULTIPLY Z COMPONENTS." */
    dxch(MPAC_AT(0u));
    ca(ADDRESS_OF(DOTINC));
    ads(ADDRESS_OF(ADDRWD));
    tc_at(DOTSUB + 22u);
    dmpsub();
    enddot();
}

/* ENDDOT: "LEAVE FINAL ACCUMULATION IN MPAC." */
void enddot(void)
{
    dxch(ADDRESS_OF(BUF) + 1u);
    das(MPAC_AT(1u));
    ad_at(MPAC_AT(0u));
    ad_at(ADDRESS_OF(BUF));
    if (ts(MPAC_AT(0u))) {
        tc_at(ENDDOT + 6u); /* "ON OVERFLOW HERE." */
        overflow();
    }
    tc_through(ADDRESS_OF(DOTRET));
}

/* ======================================================================
 * Polynomials
 * ====================================================================== */

/* POWRSERS, POLY -- LM p. 1034, CM p. 1139.
 * "DOUBLE PRECISION POLYNOMIAL EVALUATOR": A(N) X^N + ... + A(1) X + A(0)
 * for X in MPAC, by Horner's rule. POWRSERS takes the table's address
 * (less 3) in A and N-1 in L; POLY finds N-1 and the coefficients in the
 * words after its TC and returns past them ("RETURN IS TO LOC+2N+4"). */
void powrsers(void)
{
    qxch(ADDRESS_OF(POLYRET)); /* "RETURN ADDRESS" */
    ts(ADDRESS_OF(POLISH));    /* "POWER SERIES ADDRESS" */
    lxch(ADDRESS_OF(POLYCNT)); /* "N-1 TO COUNTER" */
    polycom();
}

void poly(void)
{
    ca(indexed(0u, overflow_corrected(Q)));
    ts(ADDRESS_OF(POLYCNT)); /* "N-1 TO COUNTER" */
    double_A();
    ad_at(Q_ADDRESS);
    ts(ADDRESS_OF(POLISH)); /* "L(A(N))-3 TO POLISH" */
    ad(FIVE);
    ts(ADDRESS_OF(POLYRET)); /* "STORE RETURN ADDRESS" */
    polycom();
}

/* POLYCOM: "INCOMING X WILL BE MOVED TO VBUF, SO SET ADDRWD SO DMPSUB WILL
 * MPY BY VBUF." */
static void polycom(void)
{
    set_A(LVBUF);
    ts(ADDRESS_OF(ADDRWD));
    dca(indexed(3u, POLISH));
    dxch(MPAC_AT(0u));      /* "LOAD A(N) INTO MPAC" */
    dxch(ADDRESS_OF(VBUF)); /* "SAVING X IN VBUF" */
    poly2();
}

/* POLY2, POLYLOOP: "MULTIPLY BY X", "ADD IN NEXT COEFFICIENT" ("USER'S
 * RESPONSIBILITY TO ASSURE NO OVFLOW"), and loop on POLYCNT. */
static void poly2(void)
{
    for (;;) {
        tc_at(POLY2);
        dmpsub();
        dca(indexed(1u, POLISH));
        das(MPAC_AT(0u));
        switch (ccs_at(ADDRESS_OF(POLYCNT))) {
        case POSITIVE: break;
        case IS_PLUS_ZERO:
            tc_through(ADDRESS_OF(POLYRET));
            return; /* "RETURN CALLER" */
        /* (never: POLY counts POLYCNT down to +0) */
        case NEGATIVE: dpagree(); return;
        case IS_MINUS_ZERO: dpagree_plus1(); return;
        }
        ts(ADDRESS_OF(POLYCNT)); /* POLYLOOP: "SAVE DECREMENTD LOOP COUNTER" */
        set_A(negate(TWO));
        ads(ADDRESS_OF(POLISH)); /* "REGRESS COEFFICIENT POINTER" */
    }
}

/* ======================================================================
 * Sign agreement and short multiplies
 * ====================================================================== */

/* DPAGREE, TPAGREE -- LM p. 1036, CM p. 1141.
 * "MISCELLANEOUS MULTI-PRECISION ROUTINES REQUIRED IN FIXED-FIXED BUT NOT
 * USED BY THE INTERPRETER." "FORCE SIGN AGREEMENT AMONG THE TRIPLE
 * PRECISION CONTENTS OF MPAC. RETURNING WITH SIGNUM OF THE INPUT IN A."
 * The return is in L (TPAGREE uses Q). A 1.0 of MPAC's sign is borrowed
 * from each word and paid into the next lower one. */
void dpagree(void)
{
    set_A(ZERO); /* "DOUBLE PRECISION ENTRY -- ZERO LOW-ORDER WORD" */
    dpagree_plus1();
}

static void dpagree_plus1(void)
{
    ts(MPAC_AT(2u));
    tpagree();
}

void tpagree(void)
{
    lxch(Q_ADDRESS);
    tc_at(TPAGREE + 1u);
    switch (branch()) {
    case BRANCH_POSITIVE: set_A(POSMAX); break; /* ARG+ */
    case BRANCH_ZERO:                           /* ARGZERO */
        ts(MPAC_AT(2u));                        /* "SET ALL THREE MPAC REGISTERS TO ZERO." */
        argzero2();
        return;
    case BRANCH_NEGATIVE: set_A(negate(POSMAX)); break; /* "IF NEGATIVE." */
    }
    ts(Q_ADDRESS);
    aug(A_ADDRESS); /* "FORMS +-1.0." */
    ad_at(MPAC_AT(2u));
    if (!ts(MPAC_AT(2u))) set_A(ZERO);
    ad_at(Q_ADDRESS);
    ad_at(MPAC_AT(1u));
    if (!ts(MPAC_AT(1u))) set_A(ZERO);
    ad_at(Q_ADDRESS); /* "Q STILL HAS POSMAX OR NEGMAX IN IT." */
    ad_at(MPAC_AT(0u));
    argzero2();
}

/* ARGZERO2: "ALWAYS SKIPPING UNLESS ARGZERO." "RETURN VIA L." */
static void argzero2(void)
{
    if (!ts(MPAC_AT(0u))) ts(MPAC_AT(1u));
    tc_through(L_ADDRESS);
}

/* SHORTMP -- LM p. 1036, CM p. 1141.
 * "SHORTMP MULTIPLIES THE TP CONTENTS OF MPAC BY THE SINGLE PRECISION NUMBER
 * ARRIVING IN A." */
void shortmp(void)
{
    ts(ADDRESS_OF(MPTEMP));
    shortmp_plus1();
}

void shortmp_plus1(void)
{
    mp_at(MPAC_AT(2u));
    ts(MPAC_AT(2u));
    shortmp2();
}

void shortmp2(void)
{
    set_A(ZERO); /* "SO SUBSEQUENT DAS WILL WORK." */
    xch(MPAC_AT(1u));
    dmpsub2();
}

/* DMPNSUB -- LM p. 1037, CM p. 1142.
 * "DMPNSUB MULTIPLIES THE DP FRACTION ARRIVING IN MPAC BY THE SP INTEGER
 * ARRIVING IN A. THE DP PRODUCT DEPARTS BOTH IN MPAC AND IN A AND L." */
void dmpnsub(void)
{
    ts(ADDRESS_OF(DMPNTEMP));
    mp_at(MPAC_AT(1u));
    dxch(MPAC_AT(0u)); /* "LOW PRODUCT TO MPAC, HIGH FACTOR TO A" */
    mp_at(ADDRESS_OF(DMPNTEMP));
    ca(L_ADDRESS);
    ads(MPAC_AT(0u)); /* "COMPLETING THE PRODUCT IN MPAC" */
    dca(MPAC_AT(0u)); /* "BRINGING THE PRODUCT INTO A AND L" */
    tc_through(Q_ADDRESS);
}
