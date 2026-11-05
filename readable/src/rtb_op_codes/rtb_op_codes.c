/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055), the
 * same code in both: the routines an interpretive RTB runs. So far
 * LOADTIME, SGNAGREE, CDULOGIC, 1STO2S, V1STO2S, TPMODE, DPMODE and 2V1STO2S with
 * their 1TO2SUB, and the CM's TRANSP1 and TRANSP2. */
#include "rtb_op_codes.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"
#include "../interbank/interbank.h"

static void one_to2sub(void);

/* LOADTIME -- LM p. 1397, CM p. 1508.
 * The time now (TIME2, TIME1: centiseconds, double precision) into MPAC,
 * as a DLOAD would load it: SLOAD2 clears MPAC +2 and declares the mode
 * double precision. */
void loadtime(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    sload2();               /* TCF SLOAD2 */
}

/* SGNAGREE -- LM p. 1397, CM p. 1508.
 * "FORCE TP SIGN AGREEMENT IN MPAC" (TPAGREE), back to DANZIG. */
void sgnagree(void)
{
    tc_at(SGNAGREE);
    tpagree();
    continue_at(DANZIG); /* TCF DANZIG */
}

/* CDULOGIC -- LM p. 1397, CM p. 1508.
 * "CONVERT THE SINGLE PRECISION 2'S COMPLEMENT NUMBER ARRIVING IN MPAC
 * (SCALED IN HALF-REVOLUTIONS) TO A DP 1'S COMPLEMENT NUMBER SCALED IN
 * REVOLUTIONS": a CDU angle. MPAC times HALF is the angle in revolutions;
 * a negative one also gets -HALF in its low word (CS HALF, kept in MPAC +1
 * before the product is added in), which turns the two's complement count
 * into ones' complement. "MODE IS ALREADY AT DOUBLE-PRECISION": back to
 * DANZIG. */
void cdulogic(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: set_A(ZERO); break;              /* CAF ZERO; TCF +3 */
    case IS_PLUS_ZERO: break;                       /* TCF +3 */
    case NEGATIVE:                                  /* NOOP */
    case IS_MINUS_ZERO: set_A(negate(HALF)); break; /* CS HALF */
    }
    ts(ADDRESS_OF(MPAC) + 1u);
    set_A(ZERO); /* CAF ZERO */
    xch(ADDRESS_OF(MPAC));
    mp(HALF); /* EXTEND; MP HALF */
    das(ADDRESS_OF(MPAC));
    continue_at(DANZIG);
}

/* 1STO2S -- LM p. 1397, CM p. 1509.
 * "CONVERT THE DP 1'S COMPLEMENT ANGLE SCALED IN REVOLUTIONS TO A SINGLE
 * PRECISION 2'S COMPLEMENT ANGLE SCALED IN HALF-REVOLUTIONS": 1TO2SUB
 * does it, leaving the angle in MPAC; MPAC +1 gets +0 and NEWMODE sets
 * the mode to double precision (A +0). */
void one_sto2s(void)
{
    tc_at(ONE_STO2S);
    one_to2sub();
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(MPAC) + 1u);
    newmode(); /* TCF NEWMODE */
}

/* V1STO2S -- LM p. 1397, CM p. 1509.
 * "DO 1STO2S ON A VECTOR OF ANGLES": the three DP angles in MPAC, MPAC +3
 * and MPAC +5 (in revolutions) to three single precision CDU angles in
 * two's complement, MPAC, MPAC +1 and MPAC +2. Each goes through 1TO2SUB
 * ("ANSWER ARRIVES IN A AND MPAC"), the first one's kept in MPAC +5
 * meanwhile; TPMODE declares the mode triple precision ("MODE IS TP"). */
void v1sto2s(void)
{
    tc_at(V1STO2S);
    one_to2sub();
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(V1STO2S + 3u);
    one_to2sub();
    ts(ADDRESS_OF(MPAC) + 2u);
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(V1STO2S + 7u);
    one_to2sub();
    ts(ADDRESS_OF(MPAC) + 1u);
    ca(ADDRESS_OF(MPAC) + 5u);
    ts(ADDRESS_OF(MPAC));
    set_A(ONE); /* TPMODE: CAF ONE, "MODE IS TP." */
    newmode();  /* TCF NEWMODE */
}

/* DPMODE -- LM p. 1401, CM p. 1514.
 * The mode double precision, as SLOAD2 leaves it: from SLOAD2 +2, A +0
 * stored in MPAC +2 ("SETS MPAC +2 TO ZERO IN THE PROCESS") and as the
 * MODE; back to DANZIG. */
void dpmode(void)
{
    set_A(ZERO);    /* CAF ZERO */
    sload2_plus2(); /* TCF SLOAD2 +2 */
}

/* NORMUNX1 and NORMUNIT -- LM pp. 1401-1402, CM pp. 1514-1515.
 * "RTB OP CODE NORMUNIT IS LIKE INTERPRETIVE INSTRUCTION UNIT, EXCEPT THAT
 * IT CAN BE DEPENDED ON NOT TO BLOW UP WHEN THE VECTOR BEING UNITIZED IS
 * VERY SMALL": the vector in MPAC with "SIGN AGREEMENT IN ALL COMPONENTS"
 * (VECAGREE, by BANKCALL); if the upper halves of all three are zero,
 * each is shifted "LEFT 13" (times BIT14, added back in by DAS, which
 * "GAINS A LITTLE ACCURACY") and the shift count, 13, goes to 37D of the
 * work area (NOSHIFT: else +0); then UNIT past its own VECAGREE (OFFTUNIT,
 * by POSTJUMP to UNIT +1), which ends at DANZIG. NORMUNX1 leaves the count
 * one word on, in X1 (38D). */
static void normunit_plus1(void);
static void offtunit_minus2(void);

/* CCS: +0 or -0 goes on (TCF +2, or the next word), the rest to NOSHIFT. */
static bool upper_half_zero(unsigned address)
{
    enum sign sign = ccs_at(address);

    return sign == IS_PLUS_ZERO || sign == IS_MINUS_ZERO;
}

void normunx1(void)
{
    set_A(ONE);       /* CAF ONE */
    normunit_plus1(); /* TCF NORMUNIT +1 */
}

void normunit(void)
{
    set_A(ZERO);      /* CAF ZERO */
    normunit_plus1(); /* (on into NORMUNIT +1) */
}

static void normunit_plus1(void)
{
    ad_at(ADDRESS_OF(FIXLOC));
    ts(MPAC_AT(2u)); /* the work area, one word on for NORMUNX1 */
    tc_at(NORMUNIT + 3u);
    bankcall(); /* "GET SIGN AGREEMENT IN ALL COMPONENTS" (CADR VECAGREE) */
    vecagree();
    swreturn();
    /* CCS MPAC, MPAC +3 and MPAC +5 in turn: an upper half other than
     * zero goes to NOSHIFT. */
    if (!upper_half_zero(MPAC_AT(0u)) || !upper_half_zero(MPAC_AT(3u)) ||
        !upper_half_zero(MPAC_AT(5u))) {
        set_A(ZERO); /* NOSHIFT: CAF ZERO, TCF OFFTUNIT -2 */
        offtunit_minus2();
        return;
    }
    ca(MPAC_AT(1u)); /* "SHIFT ALL COMPONENTS LEFT 13" */
    mp(BIT14);
    das(MPAC_AT(0u)); /* "DAS GAINS A LITTLE ACCURACY" */
    ca(MPAC_AT(4u));
    mp(BIT14);
    das(MPAC_AT(3u));
    ca(MPAC_AT(6u));
    mp(BIT14);
    das(MPAC_AT(5u));
    set_A(THIRTEEN);
    offtunit_minus2();
}

/* OFFTUNIT -2: the count to 37D of the work area kept in MPAC +2 (INDEX
 * MPAC +2; TS 37D), and UNIT +1, which skips the VECAGREE just done. */
static void offtunit_minus2(void)
{
    ts(indexed(37u, fetch(MPAC_AT(2u))));
    tc_at(OFFTUNIT);
    postjump(); /* CADR UNIT +1 */
    unit_plus1();
}

/* TPMODE -- LM p. 1398, CM p. 1509.
 * "MODE IS TP.": MODE 1 (NEWMODE), back to DANZIG. */
void tpmode(void)
{
    set_A(ONE); /* CAF ONE */
    newmode();  /* TCF NEWMODE */
}

/* 2V1STO2S -- LM p. 1398, CM p. 1509.
 * "V1STO2S FOR 2 COMPONENT VECTOR. USED BY RR": the two angles in MPAC and
 * MPAC +3 (DP, in revolutions) to two single precision CDU angles in two's
 * complement, MPAC and MPAC +1, the mode double precision. Each goes
 * through 1TO2SUB, the first one's answer kept in MPAC +3 meanwhile. */
void two_v1sto2s(void)
{
    tc_at(TWO_V1STO2S);
    one_to2sub();
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(TWO_V1STO2S + 3u);
    one_to2sub();
    ts(L_ADDRESS);
    ca(ADDRESS_OF(MPAC) + 3u);
    sload2(); /* TCF SLOAD2 */
}

/* 1TO2SUB -- LM p. 1398, CM p. 1509.
 * "SUBROUTINE TO DO DOUBLING AND 1'S TO 2'S CONVERSION": MPAC, doubled
 * (revolutions to half-revolutions), as one word in two's complement, to
 * MPAC and A. The CCS table makes a negative angle's two's complement form
 * ("THIS WAS REVERSE OF MSU"); a doubling that overflowed gets the limit
 * of its sign added back in (INDEX A; CAF LIMITS). "FINAL MPAC +1
 * UNSPECIFIED." */
static void one_to2sub(void)
{
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ad(ONE); break; /* AD ONE; TCF +2 */
    case IS_PLUS_ZERO: break;      /* TCF +2 */
    case NEGATIVE: com(); break;   /* COM: "THIS WAS REVERSE OF MSU." */
    case IS_MINUS_ZERO: break;
    }
    if (ts(ADDRESS_OF(MPAC))) { /* "AND SKIP ON OVERFLOW." */
        /* "OVERFLOW UNCORRECT AND IN MSU." */
        ca(indexed(ADDRESS_OF(LIMITS), a_word()));
        ads(ADDRESS_OF(MPAC));
    }
    tc_through(Q_ADDRESS); /* TC Q */
}

#if !LUMINARY099
/* TRANSP1, TRANSP2 -- CM p. 1513.
 * "EACH ROUTINE TAKES A 3X3 MATRIX STORED IN DOUBLE PRECISION IN A FIXED
 * AREA OF ERASABLE MEMORY AND REPLACES IT WITH THE TRANSPOSE MATRIX.
 * TRANSP1 USES LOCATIONS XNB+0,+1 THROUGH XNB+16D, 17D AND TRANSP2 USES
 * LOCATIONS XNB1+0,+1 THROUGH XNB1+16D, 17D. EACH MATRIX IS STORED BY
 * ROWS." The diagonal stays; each of the three pairs across it swaps
 * through A,L, three DXCHes a pair: rows 1 and 2's (+2 and +6), 1 and 3's
 * (+4 and +12D), 2 and 3's (+10D and +14D). Its EBANK set from the
 * matrix's ECADR, the A,L left with that ECADR and the L it came with. */
static void transpose(unsigned matrix)
{
    dxch(matrix + 2u);
    dxch(matrix + 6u);
    dxch(matrix + 2u);

    dxch(matrix + 4u);
    dxch(matrix + 12u);
    dxch(matrix + 4u);

    dxch(matrix + 10u);
    dxch(matrix + 14u);
    dxch(matrix + 10u);
    continue_at(DANZIG); /* TCF DANZIG */
}

void transp1(void)
{
    set_A(XNBEB); /* CAF XNBEB */
    ts(EBANK_ADDRESS);
    transpose(ADDRESS_OF(XNB));
}

void transp2(void)
{
    set_A(XNB1EB); /* CAF XNB1EB */
    ts(EBANK_ADDRESS);
    transpose(ADDRESS_OF(XNB1));
}
#endif
