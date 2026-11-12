/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099, pp. 1397-1402) and RT8_OP_CODES.agc
 * (Comanche055, pp. 1508-1516), the same code in both: the basic routines an
 * interpretive RTB runs (return to basic: RTB names one of these, the
 * interpreter's SWCALL goes there, and the routine ends back at DANZIG, the
 * interpreter's next order). The CM's file adds READPIPS, VECMODE, INCRCDUS,
 * TRANSP1 and TRANSP2, and keeps QUALITY1 and QUALITY2 (quality.c), which
 * the LM has in ORBITAL_INTEGRATION. The faithful C is
 * c64-port/src/lm_rtb_op_codes.c and cm_rtb_op_codes.c.
 *
 * Each routine here ends where the listing leaves the file: at SLOAD2,
 * NEWMODE or VMODE (which set the mode and go on to DANZIG), at DANZIG,
 * TPAGREE, BANKCALL or POSTJUMP. Where it calls a routine of the file
 * (1TO2SUB, CDUINC) it runs it and goes on; where it calls one of another
 * file (TC TPAGREE, TC BANKCALL), the word after the call is a routine of
 * its own (sgnagree_after_tpagree and the like), where that one returns. */
#include "rtb_op_codes.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static void one_to2sub(void);

/* TC Q, the return to the word in Q: its address bits (an overflow in Q,
 * its 16th bit, is not part of the address). Not modelled: a Q that is
 * not an address of fixed memory (the AGC would run it as another
 * instruction, or TC to a register). */
static void return_through_q(void)
{
    word to = (word)(Q & 07777u);

    if (Q < 010u || (Q & 070000u)) not_modelled();
    tc_through(Q_ADDRESS);
    Z = to;
}

/* LOADTIME -- LM p. 1397, CM p. 1508.
 * "LOAD TIME2, TIME1 INTO MPAC": the time now (centiseconds, double
 * precision), as a DLOAD would load it: SLOAD2 clears MPAC +2 and declares
 * the mode double precision. */
void loadtime(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    continue_at(K_SLOAD2);  /* TCF SLOAD2 */
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

#if !LUMINARY099
/* READPIPS -- CM p. 1508.
 * "READ THE PIPS INTO MPAC WITHOUT CHANGING THEM": the three PIPA counts,
 * read with the interrupts held off so that they are of one moment, into
 * the high words of the vector in MPAC, the low words +0; on to VECMODE. */
void readpips(void)
{
    inhint();
    ca(ADDRESS_OF(PIPAX));
    ts(MPAC_AT(0u));
    ca(ADDRESS_OF(PIPAY));
    ts(MPAC_AT(3u));
    ca(ADDRESS_OF(PIPAZ));
    relint();
    ts(MPAC_AT(5u));
    set_A(ZERO); /* CAF ZERO */
    ts(MPAC_AT(1u));
    ts(MPAC_AT(4u));
    ts(MPAC_AT(6u));
    vecmode();
}

/* VECMODE -- CM p. 1508: the mode vector (VMODE), back to DANZIG. */
void vecmode(void)
{
    continue_at(K_VMODE); /* TCF VMODE */
}
#endif

/* SGNAGREE -- LM p. 1397, CM p. 1508.
 * "FORCE TP SIGN AGREEMENT IN MPAC" (TPAGREE), back to DANZIG. */
void sgnagree(void)
{
    tc_to(SGNAGREE, TPAGREE);
}

/* SGNAGREE +1, where TPAGREE returns. */
void sgnagree_after_tpagree(void)
{
    continue_at(DANZIG); /* TCF DANZIG */
}

/* 1STO2S -- LM p. 1397, CM p. 1509.
 * "CONVERT THE DP 1'S COMPLEMENT ANGLE SCALED IN REVOLUTIONS TO A SINGLE
 * PRECISION 2'S COMPLEMENT ANGLE SCALED IN HALF-REVOLUTIONS": 1TO2SUB
 * does it, leaving the angle in MPAC. */
void one_sto2s(void)
{
    tc_at(ONE_STO2S);
    one_to2sub();
    one_sto2s_plus1();
}

/* 1STO2S +1, where 1TO2SUB returns: MPAC +1 gets +0, and NEWMODE sets the
 * mode to double precision (A +0). */
void one_sto2s_plus1(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(MPAC) + 1u);
    continue_at(K_NEWMODE); /* TCF NEWMODE */
}

/* V1STO2S -- LM p. 1397, CM p. 1509.
 * "DO 1STO2S ON A VECTOR OF ANGLES": the three DP angles in MPAC, MPAC +3
 * and MPAC +5 (in revolutions) to three single precision CDU angles in
 * two's complement, MPAC, MPAC +1 and MPAC +2. Each goes through 1TO2SUB
 * ("ANSWER ARRIVES IN A AND MPAC"); +1, +4 and +8 are where it returns.
 * The first answer is kept in MPAC +5 meanwhile. */
void v1sto2s(void)
{
    tc_at(V1STO2S);
    one_to2sub();
    v1sto2s_plus1();
}

void v1sto2s_plus1(void)
{
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(V1STO2S + 3u);
    one_to2sub();
    v1sto2s_plus4();
}

void v1sto2s_plus4(void)
{
    /* An answer that overflowed (1TO2SUB's ADS) skips the DXCH. */
    if (!ts(ADDRESS_OF(MPAC) + 2u)) dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(V1STO2S + 7u);
    one_to2sub();
    v1sto2s_plus8();
}

void v1sto2s_plus8(void)
{
    /* An answer that overflowed skips the CA: MPAC gets its sign (A +1
     * or -1, as the TS leaves it). */
    if (!ts(ADDRESS_OF(MPAC) + 1u)) ca(ADDRESS_OF(MPAC) + 5u);
    ts(ADDRESS_OF(MPAC));
    tpmode();
}

/* TPMODE -- LM p. 1398, CM p. 1509.
 * "MODE IS TP.": MODE 1 (NEWMODE), back to DANZIG. */
void tpmode(void)
{
    set_A(ONE);             /* CAF ONE */
    continue_at(K_NEWMODE); /* TCF NEWMODE */
}

/* 2V1STO2S -- LM p. 1398, CM p. 1509.
 * "V1STO2S FOR 2 COMPONENT VECTOR. USED BY RR": the two angles in MPAC and
 * MPAC +3 (DP, in revolutions) to two single precision CDU angles in two's
 * complement, MPAC and MPAC +1, the mode double precision (SLOAD2). Each
 * goes through 1TO2SUB, which returns to +1 and +4; the first answer is
 * kept in MPAC +3 meanwhile. */
void two_v1sto2s(void)
{
    tc_at(TWO_V1STO2S);
    one_to2sub();
    two_v1sto2s_plus1();
}

void two_v1sto2s_plus1(void)
{
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(TWO_V1STO2S + 3u);
    one_to2sub();
    two_v1sto2s_plus4();
}

void two_v1sto2s_plus4(void)
{
    /* An answer that overflowed skips the CA: SLOAD2 gets its sign. */
    if (!ts(L_ADDRESS)) ca(ADDRESS_OF(MPAC) + 3u);
    continue_at(K_SLOAD2); /* TCF SLOAD2 */
}

/* 1TO2SUB -- LM p. 1398, CM p. 1509.
 * "SUBROUTINE TO DO DOUBLING AND 1'S TO 2'S CONVERSION": MPAC, doubled
 * (revolutions to half-revolutions), as one word in two's complement, to
 * MPAC and A. The CCS table makes a negative angle's two's complement form
 * ("THIS WAS REVERSE OF MSU"); a doubling that overflowed gets the limit
 * of its sign added back in (INDEX A; CAF LIMITS). "FINAL MPAC +1
 * UNSPECIFIED." Back to the word after the TC (TC Q): the caller goes on
 * there. */
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
/* INCRCDUS -- CM p. 1511.
 * "SUBROUTINE TO INCREMENT CDUS": THETAD, THETAD +1 and THETAD +2 each by
 * the word of the vector in MPAC (MPAC, MPAC +3, MPAC +5, "INCREMENT IN 1S
 * COMPL."), through CDUINC with BUF naming the angle ("PLACE ADRES(THETA)
 * IN BUF"; LOCTHETA is ADRES THETAD); +4, +7 and +10 are where CDUINC
 * returns. Then on to VECMODE. */
void incrcdus(void)
{
    set_A(LOCTHETA); /* CAF LOCTHETA */
    ts(ADDRESS_OF(BUF));
    ca(MPAC_AT(0u)); /* CAE MPAC */
    tc_at(INCRCDUS + 3u);
    if (cduinc() == INCRCDUS + 4u) incrcdus_plus4();
}

void incrcdus_plus4(void)
{
    incr(ADDRESS_OF(BUF));
    ca(MPAC_AT(3u));
    tc_at(INCRCDUS + 6u);
    if (cduinc() == INCRCDUS + 7u) incrcdus_plus7();
}

void incrcdus_plus7(void)
{
    incr(ADDRESS_OF(BUF));
    ca(MPAC_AT(5u));
    tc_at(INCRCDUS + 011u);
    if (cduinc() == INCRCDUS + 012u) incrcdus_plus10();
}

void incrcdus_plus10(void)
{
    vecmode(); /* TCF VECMODE */
}
#endif

/* INDEX k before an instruction on erasable memory (its code and quarter
 * code in bits 11-15, an address of 10 bits): the address the instruction
 * then names. Not modelled: an index that makes it another instruction. */
static unsigned indexed_by(unsigned k, word instruction)
{
    word indexed = indexed_instruction(instruction, fetch(k));

    if ((indexed & 076000u) != (instruction & 076000u)) not_modelled();
    return indexed & 01777u;
}

/* A TS into the address an INDEX made. Not modelled: one into the bank
 * registers or Z, which the program never makes. */
static bool ts_indexed(unsigned address)
{
    if (address >= 3u && address <= 6u) not_modelled();
    return ts(address);
}

#define CCS_CODE 010000u
#define TS_CODE  054000u
#define TS_37D   (TS_CODE | 37u)

/* CDUINC -- LM p. 1398, CM p. 1511.
 * "THE FOLLOWING ROUTINE INCREMENTS IN 2S COMPLEMENT THE REGISTER WHOSE
 * ADDRESS IS IN BUF BY THE 1S COMPL. QUANTITY FOUND IN TEM2. THIS MAY BE
 * USED TO INCREMENT DESIRED IMU AND OPTICS CDU ANGLES OR ANY OTHER 2S
 * COMPL. (+0 UNEQUAL TO -0) QUANTITY. MAY BE CALLED BY BANKCALL/SWCALL."
 * The increment arrives in A ("STORE IT" in TEM2). The CCS table makes
 * the angle one's complement ("OVERFLOW HERE IF 2S COMPL. IS 180 DEG."),
 * the increment is added (the result may move from the second quadrant
 * to the third, or back), and a second CCS table takes the sum "BACK TO 2S COMPL.". If
 * that overflowed, TS keeps only the "14BIT QUANTITY WITH PRESENT SIGN",
 * and INDEX A; CAF LIMITS puts the lost bit back ("FIX IT,BY ADDING IN
 * 37777 OR 40000"). "STORE NEW ANGLE IN 2S COMPLEMENT", back through Q;
 * the Q it went back through is returned, for the caller to go on only
 * if that is the word after its TC (a BUF that names Q itself stores the
 * angle there). Two overflows the program never makes run on as the AGC
 * would: a TS TEM2 that overflows skips the INDEX BUF (so the CCS is of
 * A), and a TS of the new angle that overflows skips the TC Q, into
 * PULSEIMU: 0 then, the run gone on there. */
word cduinc(void)
{
    word back;
    bool skipped = ts(ADDRESS_OF(TEM2)); /* "1S COMPL.QUANT. ARRIVES IN ACC." */
    unsigned angle = skipped ? A_ADDRESS : indexed_by(ADDRESS_OF(BUF), CCS_CODE);

    switch (ccs_at(angle)) {       /* "CHANGE 2S COMPL. ANGLE(IN BUF)INTO 1S" */
    case POSITIVE: ad(ONE); break; /* AD ONE; TCF +4 */
    case IS_PLUS_ZERO: break;      /* TCF +4 */
    case NEGATIVE:
        ad(ONE);
        /* fall through */
    case IS_MINUS_ZERO:
        ad(ONE); /* "OVERFLOW HERE IF 2S COMPL. IS 180 DEG." */
        com();
        break;
    }
    ad_at(ADDRESS_OF(TEM2));
    switch (ccs_register(A_ADDRESS)) { /* "BACK TO 2S COMPL." */
    case POSITIVE: ad(ONE); break;     /* AD ONE; TCF +2 */
    case IS_PLUS_ZERO: break;          /* TCF +2 */
    case NEGATIVE: com(); break;
    case IS_MINUS_ZERO: break;
    }
    if (ts(ADDRESS_OF(TEM2))) {                    /* "STORE 14BIT QUANTITY WITH PRESENT SIGN" */
        ca(indexed(ADDRESS_OF(LIMITS), a_word())); /* "FIX IT,BY ADDING IN 37777 OR 40000" */
        ad_at(ADDRESS_OF(TEM2));
    }
    if (ts_indexed(
            indexed_by(ADDRESS_OF(BUF), TS_CODE))) { /* "STORE NEW ANGLE IN 2S COMPLEMENT." */
        pulseimu();
        return 0u;
    }
    back = Q;
    return_through_q();
    return back;
}

/* PULSEIMU -- LM p. 1400, CM p. 1512.
 * "RTB TO TORQUE GYROS, EXCEPT FOR THE CALL TO IMUSTALL. ECADR OF COMMANDS
 * ARRIVES IN X1." IMUPULSE, by BANKCALL, with that ECADR in A. */
void pulseimu(void)
{
    ca(indexed(ADDRESS_OF(X1), fetch(ADDRESS_OF(FIXLOC)))); /* INDEX FIXLOC; CA X1 */
    tc_to(PULSEIMU + 2u, BANKCALL);                         /* CADR IMUPULSE */
}

/* PULSEIMU +4, where IMUPULSE returns: back to DANZIG. */
void pulseimu_after_imupulse(void)
{
    continue_at(DANZIG); /* TCF DANZIG */
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

/* SIGNMPAC -- LM p. 1401, CM p. 1514.
 * "THE SUBROUTINE SIGNMPAC SETS C(MPAC, MPAC +1) TO SIGN(MPAC)": DPOSMAX
 * for a positive MPAC or +0, its complement for a negative one or -0
 * ("ALSO +0 YIELDS POSMAX AND -0 YIELDS NEGMAX"). Only MPAC is looked at;
 * the mode becomes double precision, back to DANZIG ("RETURN IS TO THE
 * NEXT INTERPRETIVE INSTRUCTION IN THE CALLING SEQUENCE"). */
void signmpac(void)
{
    dca(DPOSMAX); /* EXTEND; DCA DPOSMAX */
    dxch(ADDRESS_OF(MPAC));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: dpmode(); break;                       /* DPMODE */
    case IS_PLUS_ZERO: continue_at(K_SLOAD2 + 2u); break; /* TCF SLOAD2 +2 */
    case NEGATIVE:                                        /* TCF +1 */
    case IS_MINUS_ZERO:
        dcs(DPOSMAX);          /* EXTEND; DCS DPOSMAX */
        continue_at(K_SLOAD2); /* TCF SLOAD2 */
        break;
    }
}

/* DPMODE -- LM p. 1401, CM p. 1514.
 * The mode double precision, as SLOAD2 leaves it: from SLOAD2 +2, A +0
 * stored in MPAC +2 ("SETS MPAC +2 TO ZERO IN THE PROCESS") and as the
 * MODE; back to DANZIG. */
void dpmode(void)
{
    set_A(ZERO);                /* CAF ZERO */
    continue_at(K_SLOAD2 + 2u); /* TCF SLOAD2 +2 */
}

/* NORMUNX1 and NORMUNIT -- LM pp. 1401-1402, CM pp. 1514-1515.
 * "RTB OP CODE NORMUNIT IS LIKE INTERPRETIVE INSTRUCTION UNIT, EXCEPT THAT
 * IT CAN BE DEPENDED ON NOT TO BLOW UP WHEN THE VECTOR BEING UNITIZED IS
 * VERY SMALL": the vector in MPAC with "SIGN AGREEMENT IN ALL COMPONENTS"
 * (VECAGREE, by BANKCALL), which returns to NORMUNIT +5. NORMUNX1 starts
 * from one word on, so that the shift count is left in X1 (38D) in place
 * of 37D. */
static void normunit_plus1(void);

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
    if (ts(MPAC_AT(2u))) not_modelled(); /* the work area, one word on for NORMUNX1 */
    tc_to(NORMUNIT + 3u, BANKCALL); /* "GET SIGN AGREEMENT IN ALL COMPONENTS" (CADR VECAGREE) */
}

/* CCS: +0 or -0 goes on (TCF +2, or the next word), the rest to NOSHIFT. */
static bool upper_half_zero(unsigned address)
{
    enum sign sign = ccs_at(address);

    return sign == IS_PLUS_ZERO || sign == IS_MINUS_ZERO;
}

static void offtunit_minus2(void);

/* NORMUNIT +5, where VECAGREE returns: if the upper halves of all three
 * components are zero, each is shifted "LEFT 13" (times BIT14, added back
 * in by DAS, which "GAINS A LITTLE ACCURACY") and the shift count is 13;
 * else (NOSHIFT) +0. "IF NORMUNIT IS USED AND THE UPPER ORDER HALVES OF
 * ALL COMPONENTS ARE ZERO, THE MAGNITUDE RETURNED IN 36D WILL BE TOO LARGE
 * BY A FACTOR OF 2(13)". */
void normunit_after_vecagree(void)
{
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
 * MPAC +2; TS 37D), and UNIT +1 by POSTJUMP, past the TC VECAGREE UNIT
 * starts with, done already. */
static void offtunit_minus2(void)
{
    if (ts_indexed(indexed_by(MPAC_AT(2u), TS_37D))) not_modelled();
    tc_to(OFFTUNIT, POSTJUMP); /* CADR UNIT +1 */
}

/* VECSGNAG -- LM p. 1402, CM p. 1515.
 * "RTB VECSGNAG ...FORCES SIGN AGREEMENT OF VECTOR IN MPAC." (VECAGREE, by
 * BANKCALL), which returns to VECSGNAG +2. */
void vecsgnag(void)
{
    tc_to(VECSGNAG, BANKCALL); /* CADR VECAGREE */
}

/* VECSGNAG +2: back to DANZIG (TC DANZIG). */
void vecsgnag_after_vecagree(void)
{
    tc_to(VECSGNAG + 2u, DANZIG);
}
