/* SPDX-License-Identifier: MIT */
/* The shift orders: INTERPRETER.agc LM pp. 1047-1056, CM pp. 1152-1161 (in
 * bank 0). The faithful C is c64-port/src/lm_interp_core.c (lmi_shortt,
 * lmi_shortv, mpacshr, mpacsrnd, vshr2, tssl2, vssr, vssl, lmi_tslc,
 * lmi_genshift, right, left, vright2, genscr1, dosshft, setround).
 *
 * "THE FOLLOWING SHORT SHIFT CODES REQUIRE NO ADDRESS WORD:
 *	1. SR1 TO SR4	SCALAR SHIFT RIGHT.
 *	2. SR1R TO SR4R	SCALAR SHIFT RIGHT AND ROUND.
 *	3. SL1 TO SL4	SCALAR SHIFT LEFT.
 *	4. SL1R TO SL4R	SCALAR SHIFT LEFT AND ROUND.
 *	5. VSR1 TO VSR8	VECTOR SHIFT RIGHT (ALWAYS ROUNDS).
 *	6. VSL1 TO VSL8	VECTOR SHIFT LEFT (NEVER ROUNDS).
 * THE FOLLOWING CODES REQUIRE AND ADDRESS WHICH MAY BE INDEXED:
 *	SR, SRR, SL, SLR, VSR, VSL. IF THE ADDRESS IS INDEXED, AND THE INDEX
 * MODIFICATION RESULTS IN A NEGATIVE SHIFT COUNT, A SHIFT OF THE ABSOLUTE
 * VALUE OF THE COUNT IS DONE IN THE OPPOSITE DIRECTION."
 *
 * A shift right by n is a multiply by BIT14 indexed by n - 1 (one word
 * later in the table of powers of two per place: BIT14, BIT13, ...), up to
 * 14 places at a time. A shift left is done one place at a time, by
 * doubling (DAS of a number with itself).
 *
 * MPTEMP holds the shift count less one, and CYR, the cycle-right
 * register, the order code: each CCS CYR (any read of it) cycles it one
 * place right, so its sign is the next bit of the code each time. */
#include "interpreter.h"
#include "../agc/names.h"

static void tssl(void);
static void tssr(void);
static void tssr_plus1(void);
static void tssr_plus2(void);
static void rightr(void);
static void mpacshr(void);
static void mpacsrnd(void);
static void vshr2(void);
static void vssr(void);
static void vssr_plus1(void);
static void vssr_plus2(void);
static void vssl_minus1(void);
static void tslc2_plus1(void);
static void endtslc(void);
static void genshft2(void);
static void right(void);
static void vright2(void);
static void setround(void);
static void genscr(void);
static void genscr_plus1(void);
static void dosshft(void);
static void right_minus(void);
static void left_minus(void);
static void left(void);
static void genscl(void);
static void round_if_asked(word tc_site);

/* INDEX k; CAF BIT14: the power of two that shifts right by C(k) + 1. */
static word shifting_bit(unsigned k)
{
    return read_word((word)indexed(ADDRESS_OF(BIT14), fetch(k)));
}

/* ======================================================================
 * Short scalar shifts
 * ====================================================================== */

/* SHORTT -- LM p. 1047, CM p. 1152.
 * "SCALAR SHORT SHIFTS COME HERE. THE SHIFT COUNT-1 IS NOW IN BITS 2-3 OF
 * CYR. THE ROUNDING BIT IS IN BIT1 AT THIS POINT." (TS SR stores them
 * shifted right one place.) Then "SEE IF RIGHT OR LEFT SHIFT DESIRED." */
void shortt(void)
{
    set_A(SIX);
    mask_at(CYR_ADDRESS);
    ts(SR_ADDRESS);

    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: tssl(); return; /* "SHIFT LEFT." */
    case IS_PLUS_ZERO:
        /* (never: CYR holds an order code. The AGC would run the next word,
         * SRDDV, DEC 20, as TC 24.) */
        tc_at(ADDRESS_OF(SRDDV));
        continue_at(SRDDV);
        return;
    case NEGATIVE: tssr(); return;
    case IS_MINUS_ZERO: tssr_plus1(); return;
    }
}

/* TSSR: the shifting bit, then "SEE IF A ROUND IS DESIRED." */
static void tssr(void)
{
    set_A(shifting_bit(SR_ADDRESS)); /* "GET SHIFTING BIT." */
    tssr_plus2();
}

/* TSSR +1: CCS's -0 slot skips the INDEX. */
static void tssr_plus1(void)
{
    set_A(BIT14);
    tssr_plus2();
}

/* TSSR +2: the shifting bit to MPTEMP and the round test. (A CYR of +0 leaves A +0 for NEWMODE: "SET
 * MODE TO DP (C(A) = 0)"; a CYR of -0 enters MPACSHR one word in, with A
 * +0 for the MP.) */
static void tssr_plus2(void)
{
    ts(ADDRESS_OF(MPTEMP));
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: rightr(); return;
    case IS_PLUS_ZERO: newmode(); return;
    case NEGATIVE: mpacshr(); return;
    case IS_MINUS_ZERO:
        mp_at(MPAC_AT(2u));
        mpacshr_plus3();
        return;
    }
}

/* RIGHTR -- LM p. 1047, CM p. 1152. "YES -- SHIFT RIGHT AND ROUND." Then
 * NEWMODE: "SET MODE TO DP (C(A) = 0)." */
static void rightr(void)
{
    tc_at(RIGHTR);
    mpacsrnd();
    newmode();
}

/* MPACSHR -- LM pp. 1047-1048, CM pp. 1152-1153.
 * "DO A TRIPLE PRECISION SHIFT RIGHT": each word of MPAC times the shifting
 * bit, from the least significant, the bits shifted out of each word (L)
 * going into the next. */
static void mpacshr(void)
{
    ca(ADDRESS_OF(MPTEMP));
    mp_at(MPAC_AT(2u));
    mpacshr_plus3();
}

/* MPACSHR +3: "(EXIT FROM SQRT AND ABVAL)." */
void mpacshr_plus3(void)
{
    ts(MPAC_AT(2u));
    ca(ADDRESS_OF(MPTEMP));
    mp_at(MPAC_AT(0u)); /* "SHIFT MAJOR PART INTO A,L AND PLACE IN */
    dxch(MPAC_AT(0u));  /* MPAC,+1." */
    ca(ADDRESS_OF(MPTEMP));
    mp_at(L_ADDRESS); /* "ORIGINAL C(MPAC +1)." */
    das(MPAC_AT(1u)); /* "GUARANTEED NO OVERFLOW." */
    continue_at(DANZIG);
}

/* MPACSRND -- LM p. 1048, CM p. 1153. "MPAC SHIFT RIGHT AND ROUND
 * SUBROUTINES." "WE HAVE TO DO ALL THREE MULTIPLIES SINCE MPAC +1 AND MPAC
 * +2 MIGHT HAVE SIGN DISAGREEMENT WITH A SHIFT RIGHT OF L." */
static void mpacsrnd(void)
{
    ca(MPAC_AT(2u));
    mp_at(ADDRESS_OF(MPTEMP));
    xch(MPAC_AT(1u));
    mp_at(ADDRESS_OF(MPTEMP));
    xch(MPAC_AT(1u)); /* "TRIAL MINOR PART." */
    ad_at(L_ADDRESS);
    vshr2();
}

/* VSHR2: "(FINISH VECTOR COMPONENT SHIFT RIGHT AND ROUND.)" The bits shifted
 * out, doubled, overflow when they are half a unit or more: the round. */
static void vshr2(void)
{
    double_A();
    if (ts(MPAC_AT(2u))) ads(MPAC_AT(1u)); /* "GUARANTEED NO OVERFLOW." */

    set_A(ZERO);
    ts(MPAC_AT(2u));
    xch(MPAC_AT(0u)); /* "SETTING TO ZERO SO FOLLOWING DAS WORKS." */
    mp_at(ADDRESS_OF(MPTEMP));
    das(MPAC_AT(0u)); /* "AGAIN NO OVERFLOW." */
    tc_through(Q_ADDRESS);
}

/* VSHRRND -- LM p. 1048, CM p. 1153. "ENTRY TO SHIFT RIGHT AND ROUND MPAC
 * WHEN MPAC CONTAINS A VECTOR COMPONENT." */
void vshrrnd(void)
{
    ca(ADDRESS_OF(MPTEMP));
    mp_at(MPAC_AT(1u));
    ts(MPAC_AT(1u));
    xch(L_ADDRESS);
    vshr2(); /* "GO ADD ONE IF NECESSARY AND FINISH." */
}

/* TSSL -- LM p. 1049, CM p. 1154. "ROUTINE FOR SHORT SCALAR SHIFT LEFT (AND
 * MAYBE ROUND)." */
static void tssl(void)
{
    ca(SR_ADDRESS); /* "GET SHIFT COUNT FOR SR." */
    tssl_plus1();
}

void tssl_plus1(void)
{
    ts(ADDRESS_OF(MPTEMP));
    tssl_plus2();
}

/* TSSL +2: "ENTRY HERE FROM SL FOR SCALARS." "SHIFTING LEFT ONE PLACE AT A
 * TIME IS FASTER THAN DOING THE WHOLE SHIFT WITH MULTIPLIES ASSUMING THAT
 * FREQUENCY OF SHIFT COUNTS GOES DOWN RAPIDLY AS A FUNCTION OF THEIR
 * MAGNITUDE." Then CYR's sign says whether to round. */
void tssl_plus2(void)
{
    for (;;) {
        dca(MPAC_AT(1u));
        das(MPAC_AT(1u));
        ad_at(MPAC_AT(0u));
        ad_at(MPAC_AT(0u));
        if (ts(MPAC_AT(0u)))
            ts(ADDRESS_OF(OVFIND)); /* "OVERFLOW. (LEAVES OVERFLOW-CORRECTED
                                     * RESULT ANYWAY)." */

        /* "LOOP ON DECREMENTED SHIFT COUNT." (The count is never
         * negative; the CCS slots for it would round, or leave.) */
        switch (ccs_at(ADDRESS_OF(MPTEMP))) {
        case POSITIVE: ts(ADDRESS_OF(MPTEMP)); continue;
        case IS_PLUS_ZERO: break;
        case NEGATIVE: round_(); return;
        case IS_MINUS_ZERO: continue_at(DANZIG); return;
        }
        break;
    }

    switch (ccs_at(CYR_ADDRESS)) {              /* "SEE IF ROUND WANTED." */
    case POSITIVE: round_(); return;            /* "YES -- ROUND AND EXIT." */
    case IS_PLUS_ZERO:                          /* "SL LEAVES A ZERO IN CYR FOR NO ROUND." */
    case NEGATIVE: continue_at(DANZIG); return; /* "NO -- EXIT IMMEDIATELY" */
    case IS_MINUS_ZERO:
        /* CYR's -0 (GENSCL makes it for an unrounded count) passes the
         * three exits and runs on into SHORTV. */
        shortv();
        return;
    }
}

/* ROUND -- LM p. 1049, CM p. 1154. The unary order ROUND, which TSSL
 * shares: "ROUND AND EXIT." */
void round_(void)
{
    tc_at(ROUND);
    roundsub();
    continue_at(DANZIG);
}

/* ======================================================================
 * Vector shifts
 * ====================================================================== */

/* SHORTV -- LM p. 1050, CM p. 1155. "VECTOR SHIFTING ROUTINES."
 * "SAVE 3 BIT SHIFT COUNT -- 1 WITHOUT EDITING CYR" (MASK reads it without
 * cycling it), then "SEE IF LEFT OR RIGHT SHIFT." */
void shortv(void)
{
    set_A(LOW3);
    mask_at(CYR_ADDRESS);
    ts(ADDRESS_OF(MPTEMP));

    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: vssl(); return; /* "VECTOR SHIFT LEFT." */
    case IS_PLUS_ZERO:
        /* (never: CYR holds an order code. The next word, OCT176, "USED IN
         * PROCESSED SHIFTS WITH - COUNT", would run as TC 176.) */
        tc_at(ADDRESS_OF(OCT176));
        continue_at(OCT176);
        return;
    case NEGATIVE: vssr(); return;
    case IS_MINUS_ZERO: vssr_plus1(); return;
    }
}

/* VSSR -- LM p. 1050, CM p. 1155. "(ENTRY FROM SR). PICK UP SHIFTING BIT.
 * MPTEMP CONTAINS THE SHIFT COUNT - 1." Each component in turn is swapped
 * into MPAC and shifted right and rounded. */
static void vssr(void)
{
    set_A(shifting_bit(ADDRESS_OF(MPTEMP)));
    vssr_plus2();
}

/* VSSR +1: CCS's -0 slot skips the INDEX (a shift of one). */
static void vssr_plus1(void)
{
    set_A(BIT14);
    vssr_plus2();
}

static void vssr_plus2(void)
{
    ts(ADDRESS_OF(MPTEMP));
    tc_at(VSSR + 3u);
    vshrrnd(); /* "SHIFT X COMPONENT." */

    dxch(MPAC_AT(0u)); /* "SWAP X AND Y COMPONENTS." */
    dxch(MPAC_AT(3u));
    dxch(MPAC_AT(0u));
    tc_at(VSSR + 7u);
    vshrrnd(); /* "SHIFT Y COMPONENT." */

    dxch(MPAC_AT(0u)); /* "SWAP Y AND Z COMPONENTS." */
    dxch(MPAC_AT(5u));
    dxch(MPAC_AT(0u));
    tc_at(VSSR + 11u);
    vshrrnd(); /* "SHIFT Z COMPONENT." */

    vrotatex(); /* "RESTORE COMPONENTS TO PROPER PLACES." */
}

/* VSSL -- LM p. 1051, CM p. 1156. "VECTOR SHIFT LEFT -- DONE ONE PLACE AT A
 * TIME." Each component doubled; an overflow sets the component to the
 * largest number of its sign, and OVFIND. */
void vssl(void)
{
    for (;;) {
        dca(MPAC_AT(0u));
        das(MPAC_AT(0u));
        if (!A_is_zero()) {
            tc_at(VSSL + 5u);
            overflow();
        }

        dca(MPAC_AT(3u));
        das(MPAC_AT(3u));
        if (!A_is_zero()) {
            tc_at(VSSL + 11u);
            overflwy();
        }

        dca(MPAC_AT(5u));
        das(MPAC_AT(5u));
        if (!A_is_zero()) {
            tc_at(VSSL + 17u);
            overflwz();
        }

        /* "LOOP ON DECREMENTED SHIFT COUNTER." (A negative count, which
         * never comes, would run on into TSLC2.) */
        switch (ccs_at(ADDRESS_OF(MPTEMP))) {
        case POSITIVE: vssl_minus1(); continue;         /* VSSL -1 */
        case IS_PLUS_ZERO: continue_at(DANZIG); return; /* "EXIT." */
        case NEGATIVE: tslc2(); return;
        case IS_MINUS_ZERO: tslc2_plus1(); return;
        }
    }
}

/* VSSL -1: "SHIFTING LOOP." */
static void vssl_minus1(void)
{
    ts(ADDRESS_OF(MPTEMP));
}

/* ======================================================================
 * TSLC
 * ====================================================================== */

/* TSLC2 -- LM p. 1052, CM p. 1157. "TSLC -- TRIPLE SHIFT LEFT AND COUNT.
 * SHIFTS MPAC LEFT UNTIL GREATER THAN .5 IN MAGNITUDE, LEAVING THE
 * COMPLEMENT OF THE NUMBER OF SHIFTS REQUIRED IN X." */
void tslc2(void)
{
    ts(ADDRESS_OF(MPTEMP)); /* "START BY ZEROING SHIFT COUNT (IN A NOW)." */
    tslc2_plus1();
}

static void tslc2_plus1(void)
{
    tc_at(TSLC2 + 1u);
    if (branch() == BRANCH_ZERO) {
        endtslc(); /* "EXIT WITH NO SHIFTING IF ARGUMENT ZERO." */
        return;
    }

    tc_at(TSLC2 + 4u);
    tpagree(); /* "MAY CAUSE UPSHIFT OF ONE EXTRA PLACE." */

    ca(MPAC_AT(0u)); /* "BEGIN NORMALIZATION LOOP." */
    for (;;) {
        double_A(); /* TSLCTEST: "SEE IF (ANOTHER) SHIFT IS REQUIRED" */
        if (A_has_overflow()) break;

        incr(ADDRESS_OF(MPTEMP)); /* TSLCLOOP: "INCREMENT SHIFT COUNTER." */
        dca(MPAC_AT(1u));
        das(MPAC_AT(1u));
        ad_at(MPAC_AT(0u));
        ads(MPAC_AT(0u));
    }
    endtslc();
}

/* ENDTSLC: "STORE SHIFT COUNT AND RETURN TO DANZIG." */
static void endtslc(void)
{
    cs(ADDRESS_OF(MPTEMP));
    store1();
}

/* ======================================================================
 * The general shifts: SR, SRR, SL, SLR, VSR, VSL
 * ====================================================================== */

/* GENSHIFT -- LM p. 1053, CM p. 1158.
 * "THE GIVEN ADDRESS IS DECODED AS FOLLOWS:
 *	BITS 1-7	SHIFT COUNT (SUBADDRESS) LESS THAN 125 DECIMAL.
 *	BIT 8		PSEUDO SIGN BIT (DETECTS CHANGE IN SIGN IN INDEXED SHIFTS).
 *	BIT 9		0 FOR LEFT SHIFT, AND 1 FOR RIGHT SHIFT.
 *	BIT 10		1 FOR TERMINAL ROUND ON SCALAR SHIFTS, 0 OTHERWISE"
 * "GET SHIFT COUNT, TESTING FOR ZERO. (ARRIVES WITH C(A) = LOW7)." */
void genshift(void)
{
    mask_at(ADDRESS_OF(ADDRWD));
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* (the count is 0 to 177) */
        genshft2();                      /* "IF NON-ZERO, PROCEED WITH DECREMENTED CT" */
        return;
    }
    /* "ZERO SHIFT COUNT. NO SHIFTS NEEDED BUT WE MIGHT HAVE TO ROUND MPAC
     * ON SLR AND SRR (SCALAR ONLY)." */
    round_if_asked(GENSHIFT + 6u);
}

/* CAF BIT10; MASK ADDRWD; CCS A; TC ROUNDSUB; TCF DANZIG: the round of
 * SRR and SLR, which the listing writes twice (GENSHIFT +3 and SLOPEHI
 * +1). */
static void round_if_asked(word tc_site)
{
    set_A(BIT10);
    mask_at(ADDRESS_OF(ADDRWD));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(tc_site);
        roundsub();
    }
    continue_at(DANZIG);
}

/* GENSHFT2: "DECREMENTED SHIFT COUNT TO MPTEMP." Then "TEST MEANING OF LOW
 * SEVEN BIT COUNT": bits 8 and 9 of the address (the product's upper
 * word) pick one of four ways (INDEX A; TCF +1). */
static void genshft2(void)
{
    ts(ADDRESS_OF(MPTEMP));
    set_A(BIT8);
    mp_at(ADDRESS_OF(ADDRWD));
    mask(LOW2);
    switch (a_word()) {
    case 0: right_minus(); return; /* "NEGATIVE SHIFT COUNT FOR SL OR SLR." */
    case 1: left(); return;        /* "SL OR SLR." */
    case 2: left_minus(); return;  /* "NEGATIVE SHIFT COUNT WITH SR OR SRR." */
    default: right(); return;      /* SR or SRR */
    }
}

/* RIGHT -- LM p. 1054, CM p. 1159. "GENERAL SHIFT RIGHT". "SET IF VECTOR
 * OR SCALAR." */
static void right(void)
{
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: genscr(); return;
    case NEGATIVE: ca(ADDRESS_OF(MPTEMP)); break; /* "SEE IF SHIFT COUNT LESS THAN 14D." */
    case IS_MINUS_ZERO: break;                    /* (never; would enter VRIGHT2 with A +0) */
    }
    vright2();
}

/* VRIGHT2: a vector shift right of 14 or more is done 14 places at a time,
 * a word: each component's major word goes into its minor word ("THE
 * ROUND AT THIS STAGE MAY INTRODUCE A ONE BIT ERROR IN A SHIFT RIGHT
 * 15D"), and the rest by VSSR. */
static void vright2(void)
{
    for (;;) {
        ad(NEG12);
        if (A_is_zero_or_minus()) {
            vssr(); /* "IF SO, BRANCH AND SHIFT IMMEDIATELY." */
            return;
        }

        ad(NEGONE);             /* "IF NOT, REDUCE MPTEMP BY A TOTAL OF 14. */
        ts(ADDRESS_OF(MPTEMP)); /* AND DO A SHIFT RIGHT AND ROUND BY 14." */
        set_A(ZERO);
        ts(L_ADDRESS);
        xch(MPAC_AT(0u));
        xch(MPAC_AT(1u));
        tc_at(VRIGHT2 + 9u);
        setround();       /* "X COMPONENT NOW SHIFTED, SO MAKE UP THE */
        das(MPAC_AT(0u)); /* ROUNDING QUANTITY (0 IN A AND 0 OR +-1 IN L)." */

        xch(MPAC_AT(3u)); /* "REPEAT THE ABOVE PROCESS FOR Y AND Z." */
        xch(MPAC_AT(4u));
        tc_at(VRIGHT2 + 13u);
        setround();
        das(MPAC_AT(3u)); /* "NO OVERFLOW ON THESE ADDS." */

        xch(MPAC_AT(5u));
        xch(MPAC_AT(6u));
        tc_at(VRIGHT2 + 17u);
        setround();
        das(MPAC_AT(5u));

        /* "SEE IF DONE, DOING FINAL DECREMENT." The AD NEGONE above makes
         * an exact count -0, the exit. (A negative count never comes: the
         * AGC would run the next word, BIASLO, as an instruction.) */
        switch (ccs_at(ADDRESS_OF(MPTEMP))) {
        case POSITIVE: ts(ADDRESS_OF(MPTEMP)); continue;
        case IS_PLUS_ZERO: continue;
        case NEGATIVE:
            tc_at(ADDRESS_OF(BIASLO)); /* DEC .2974 B-1 is TC 4604 */
            continue_at(BIASLO);
            return;
        case IS_MINUS_ZERO: continue_at(DANZIG); return;
        }
    }
}

/* SETROUND -- LM p. 1054, CM p. 1159. "MAKES UP ROUNDING QUANTITY FROM
 * ARRIVING C(A). L IS ZERO INITIALLY." "RETURN AND DO THE DAS, RESETTING L
 * TO 0." */
static void setround(void)
{
    double_A();
    if (!ts(MPAC_AT(2u))) set_A(ZERO);
    xch(L_ADDRESS);
    tc_through(Q_ADDRESS);
}

/* GENSCR -- LM p. 1055, CM p. 1160. "PROCESS SR AND SRR FOR SCALARS."
 * "SEE IF THE ORIGINAL SHIFT COUNT WAS LESS THAN 14D", and "DO THE SHIFT
 * IMMEDIATELY IF SO"; if not, shift MPAC right a whole word (14 places)
 * and try again with the rest. */
static void genscr(void)
{
    ca(ADDRESS_OF(MPTEMP));
    genscr_plus1();
}

static void genscr_plus1(void)
{
    ad(NEG12);
    if (A_is_zero_or_minus()) {
        dosshft();
        return;
    }
    genscr_plus4();
}

/* GENSCR +4: "DECREMENT SHIFT COUNT BY 14D AND SHIFT MPAC RIGHT 14 PLACES."
 * (SQRT enters here with ADDRWD zero, "TO PREVENT ROUND".) The TC back to
 * GENSCR +1 runs at most nine times (the count is under 128). */
void genscr_plus4(void)
{
    ad(NEGONE);
    ts(ADDRESS_OF(MPTEMP));
    set_A(ZERO);
    xch(MPAC_AT(0u));
    xch(MPAC_AT(1u));
    ts(MPAC_AT(2u));

    /* "SEE IF FINISHED, DO FINAL DECREMENT." As in VRIGHT2, an exact count
     * is -0, the exit. (A negative count never comes: the AGC would run the
     * constant SLOPEHI as an instruction on the way.) */
    switch (ccs_at(ADDRESS_OF(MPTEMP))) {
    case POSITIVE: ts(ADDRESS_OF(MPTEMP)); /* fall through */
    case IS_PLUS_ZERO:
        tc_at(GENSCR + 12u);
        genscr_plus1();
        return;
    case NEGATIVE:
        lxch(SLOPEHI & 01777u); /* DEC .5884 is 22650: LXCH 650 */
        /* fall through */
    case IS_MINUS_ZERO:
        round_if_asked(ADDRESS_OF(SLOPEHI) + 4u); /* "FINISHED WITH SHIFT. SEE IF ROUND WANTED." */
        return;
    }
}

/* DOSSHFT: "PICK UP SHIFTING BIT", "SEE IF TERMINAL ROUND DESIRED." */
static void dosshft(void)
{
    set_A(shifting_bit(ADDRESS_OF(MPTEMP)));
    ts(ADDRESS_OF(MPTEMP));
    set_A(BIT10);
    mask_at(ADDRESS_OF(ADDRWD));
    if (ccs_at(A_ADDRESS) == POSITIVE)
        rightr(); /* "YES." */
    else
        mpacshr(); /* "JUST SHIFT RIGHT." */
}

/* RIGHT-, LEFT-, LEFT -- LM p. 1056, CM p. 1161. "PROCESS THE RIGHT- (SL(R)
 * WITH A NEGATIVE COUNT), LEFT-, AND LEFT OPTIONS." A count whose index
 * made it negative has bit 8 set: 177 - count is its magnitude less one. */
static void right_minus(void)
{
    cs(ADDRESS_OF(MPTEMP)); /* "GET ABSOLUTE VALUE - 1 OF SHIFT COUNT */
    ad(OCT176);             /* UNDERSTANDING THAT BIT8 (PSEUDO-SIGN) */
    ts(ADDRESS_OF(MPTEMP)); /* WAS 1 INITIALLY." */
    right();                /* "DO NORMAL SHIFT RIGHT." */
}

static void left_minus(void)
{
    set_A(negate(OCT176));     /* "SAME PROLOGUE TO LEFT FOR INDEXED RIGHT */
    ad_at(ADDRESS_OF(MPTEMP)); /* SHIFT WHOSE NET SHIFT COUNT IS NEGATIVE" */
    com();
    ts(ADDRESS_OF(MPTEMP));
    left();
}

/* LEFT: "SINCE LEFT SHIFTING IS DONE ONE PLACE AT A TIME, NO COMPARISON
 * WITH 14 NEED BE DONE. FOR SCALARS, SEE IF TERMINAL ROUND DESIRED. FOR
 * VECTORS, SHIFT IMMEDIATELY." */
static void left(void)
{
    if (ccs_at(ADDRESS_OF(MODE)) == NEGATIVE)
        vssl();
    else
        genscl();
}

/* GENSCL: "PUT ROUNDING BIT (BIT 10 OF ADDRWD) INTO BIT 15 OF CYR WHERE THE
 * ROUNDING BIT OF A SHORT SHIFT LEFT WOULD BE" (stored there, CYR cycles
 * it to bit 14, and the CCS CYR after the shift to the sign). */
static void genscl(void)
{
    cs(ADDRESS_OF(ADDRWD));
    mp(BIT6);
    ts(CYR_ADDRESS);
    tssl_plus2(); /* "DO THE SHIFT." */
}
