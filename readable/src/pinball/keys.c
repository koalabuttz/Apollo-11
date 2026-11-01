/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 1: the keys -- LM pp.
 * 398-405, CM pp. 315-322. The faithful C is c64-port/src/lanec_pin_keys.c
 * and the key paths of the port's Pinball modules (src/lanec_pw_*.c).
 *
 * "START OF EXECUTIVE SECTION OF PINBALL". KEYRUPT leaves the key's code
 * in MPAC and starts CHARIN as a job. CHARIN makes the display system
 * busy and jumps to the key's routine through a table of one TC per code.
 * The digits go to NUM, which builds the verb, the noun or a data register
 * (R1, R2, R3) one character at a time: in octal three bits at a time, in
 * decimal as a sum it turns into a fraction when the register is full.
 * DSPCOUNT says which digit of the DSKY comes next (the "DSP number":
 * VD1 is the verb's left digit, ND1 the noun's, R1D1 R1's first). VERB and
 * NOUN blank their two digits and set DSPCOUNT to the first. + and - (the
 * sign keys) are taken only before the first digit of a decimal word.
 * CLEAR blanks the register being loaded, or backs up to the one before.
 *
 * All of it is in bank 40, word for word the same in both missions (the
 * CM writes DECON's 2DEC without the 1). A call to a routine of this part
 * is a C call: a subroutine that goes back with TC Q or TC L returns true
 * (the AGC is then at the word after the call), one that leaves for
 * ENDOFJOB or CCSHOLE returns false. A call to a routine of the file's
 * other parts (CHARALRM, ERROR, VBRELDSP, UPDATVB by BANKCALL, ENTER by
 * POSTJUMP) or out of the file (ENDOFJOB) ends the function with Q and Z
 * set, and the word it returns to, if any, is an entry of its own.
 * ENTERJMP, the ENTER key's word, is written with ENTER. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* The key codes' routines (CHARIN2 +4 on): "INPUT CODE / FUNCTION". */
static const word key_routine[32] = {
    CHARALRM,         /* 0 */
    NUM,              /* 1 */
    NUM,              /* 2 */
    NUM,              /* 3 */
    NUM,              /* 4 */
    NUM,              /* 5 */
    NUM,              /* 6 */
    NUM,              /* 7 */
    EIGHTY_NINE_TEST, /* 10   8 */
    EIGHTY_NINE_TEST, /* 11   9 */
    CHARALRM,         /* 12 */
    CHARALRM,         /* 13 */
    CHARALRM,         /* 14 */
    CHARALRM,         /* 15 */
    CHARALRM,         /* 16 */
    CHARALRM,         /* 17 */
    NUM - 2u,         /* 20   0 */
    VERB,             /* 21   VERB */
    ERROR,            /* 22   ERROR LIGHT RESET */
    CHARALRM,         /* 23 */
    CHARALRM,         /* 24 */
    CHARALRM,         /* 25 */
    CHARALRM,         /* 26 */
    CHARALRM,         /* 27 */
    CHARALRM,         /* 30 */
    VBRELDSP,         /* 31   KEY RELEASE */
    POSGN,            /* 32   + */
    NEGSGN,           /* 33   - */
    ENTERJMP,         /* 34   ENTER */
    CHARALRM,         /* 35 */
    CLEAR,            /* 36   CLEAR */
    NOUN,             /* 37   NOUN */
};

static bool getinrel_returns(void);
static bool legaltst_returns(void);
static bool five_blank_plus2_returns(void);

/* ---- CHARIN ---- */

/* CHARIN -- LM p. 398, CM p. 315.
 * "BLOCK DISPLAY SYST": DSPLOCK set, its old word kept in 21/22REG "FOR
 * ERROR LIGHT RESET". "ALL KEYS EXCEPT ER TURN ON KR LITE IF CADRSTOR IS
 * FULL. THIS REMINDS OPERATOR TO RE-ESTABLISH A FLASHING DISPLAY WHICH HE
 * HAS OBSCURED WITH DISPLAYS OF HIS OWN": with CADRSTOR not +0 and a key
 * other than ERROR LIGHT RESET, KEY REL goes on (RELDSPON). Then CHARIN2.
 * CCS CADRSTOR's four words are TC +2 (+), TC CHARIN2 (+0), CS ELRCODE1
 * (-) and AD MPAC (-0); a -0 adds the key to +0, so any key lights it. */
void charin(void)
{
    set_A(ONE);
    xch(ADDRESS_OF(DSPLOCK));
    ts(ADDRESS_OF(TWENTY_ONE_22REG));
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE:
        tc_at(CHARIN + 4u); /* TC +2 */
        /* fall through */
    case NEGATIVE:
        set_A(negate(ELRCODE1));
        /* fall through */
    case IS_MINUS_ZERO:
        ad_at(ADDRESS_OF(MPAC)); /* EXTEND; BZF CHARIN2 */
        if (!A_is_zero()) {
            tc_at(CHARIN + 10u);
            reldspon();
        }
        break;
    case IS_PLUS_ZERO:
        tc_at(CHARIN + 5u); /* TC CHARIN2 */
        break;
    }

    /* CHARIN2: the key's code into CHAR, MPAC back as it was (the lock's
     * old word, or its sum with the key), and INDEX A; TC +1 to the key's
     * word of the table, a TC to its routine. */
    xch(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(CHAR));
    word key = a_word();
    if (key > 037u) not_modelled(); /* KEYRUPT's codes are 5 bits */
    tc_at(CHARIN2 + 3u);
    tc(CHARIN2 + 4u + key, key_routine[key]);
}

/* ---- The number keys ---- */

/* 89TEST -- LM p. 399, CM p. 316.
 * An 8 or a 9: not with DSPCOUNT negative ("BLOCK DATA IN IF DSPCOUNT IS
 * - OR -0"), and "IF DECBRNCH IS +, 8 OR 9 OK", that is in a decimal load;
 * in an octal one (+0) an OPR ERR (CHARALRM). */
void eighty_nine_test(void)
{
    switch (ccs_at(ADDRESS_OF(DSPCOUNT))) {
    case POSITIVE: tc_at(EIGHTY_NINE_TEST + 1u); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at(EIGHTY_NINE_TEST + 2u); break; /* TC +3 */
    case NEGATIVE: tc(EIGHTY_NINE_TEST + 3u, ENDOFJOB); return;
    case IS_MINUS_ZERO: tc(EIGHTY_NINE_TEST + 4u, ENDOFJOB); return;
    }
    set_A(THREE);
    mask_at(ADDRESS_OF(DECBRNCH)); /* CCS A: MASK leaves 0 to 3 */
    if (ccs_at(A_ADDRESS) == POSITIVE)
        tc(EIGHTY_NINE_TEST + 8u, NUM);
    else
        tc(EIGHTY_NINE_TEST + 9u, CHARALRM);
}

/* NUM -2 -- LM p. 399, CM p. 316. The 0 key, whose code is 20: CHAR +0. */
void num_minus2(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CHAR));
    num();
}

/* The four words of CCS CLPASS; CAF ZERO; TS CLPASS; TC +1 at at: "IF
 * CLPASS IS + OR +0, MAKE IT +0." */
static void clpass_plus_zero(word at)
{
    switch (ccs_at(ADDRESS_OF(CLPASS))) {
    case POSITIVE:
        set_A(ZERO);
        /* fall through */
    case IS_PLUS_ZERO:
        ts(ADDRESS_OF(CLPASS));
        /* fall through */
    case NEGATIVE:
        tc_at(at + 3u); /* TC +1 */
        break;
    case IS_MINUS_ZERO: break;
    }
}

static void dectobin(void);
static void endnmtst(void);
static void decend(void);
static void endall(void);
static void mornum(void);
static void mornum_plus1(void);

/* NUM -- LM pp. 399-400, CM pp. 316-317.
 * "NUM ASSEMBLES OCTAL 3 BITS AT A TIME. FOR DECIMAL IT CONVERTS INCOMING
 * WORD AS A FRACTION, KEEPING RESULTS TO DP. OCTAL RESULTS ARE LEFT IN
 * XREG, YREG, OR ZREG. HI PART OF DEC IN XREG, YREG, ZREG. THE LOW PARTS
 * IN XREGLP, YREGLP, OR ZREGLP. DECBRNCH IS LEFT AT +0 FOR OCT, +1 FOR +
 * DEC, +2 FOR - DEC. IF DSPCOUNT WAS LEFT -, NO MORE DATA IS ACCEPTED."
 * The digit goes on the DSKY first (its relay code from RELTAB into CODE,
 * DSPCOUNT's place into COUNT, DSPIN); INREL says which register it goes
 * into (GETINREL): 0 VERBREG, 1 NOUNREG, 2 to 4 XREG to ZREG. */
void num(void)
{
    switch (ccs_at(ADDRESS_OF(DSPCOUNT))) {
    case POSITIVE: tc_at(NUM + 1u); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at(NUM + 2u); break; /* TC +3 */
    case NEGATIVE:
        tc_at(NUM + 3u); /* TC +1 */
        /* fall through */
    case IS_MINUS_ZERO: tc(NUM + 4u, ENDOFJOB); return;
    }
    tc_at(NUM + 5u);
    getinrel_returns();
    clpass_plus_zero(NUM + 6u);

    ca(indexed_by(ADDRESS_OF(RELTAB), fetch(ADDRESS_OF(CHAR))));
    mask(LOW5);
    ts(ADDRESS_OF(CODE));
    ca(ADDRESS_OF(DSPCOUNT));
    ts(ADDRESS_OF(COUNT));
    tc_at(NUM + 16u);
    if (!dspin_returns()) return;

    set_A(THREE);
    mask_at(ADDRESS_OF(DECBRNCH)); /* "+0, OCTAL. +1, + DEC. +2, - DEC." */
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(NUM + 20u);
        dectobin();
        return;
    }
    /* Octal: the register three bits up (CYL three times), the digit
     * added. */
    xch(indexed_erasable(ADDRESS_OF(VERBREG), fetch(ADDRESS_OF(INREL))));
    ts(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    xch(ADDRESS_OF(CYL));
    ad_at(ADDRESS_OF(CHAR));
    tc_at(DECTOBIN - 1u);
    endnmtst();
}

/* DECTOBIN -- LM p. 400, CM p. 317.
 * Decimal: the register's sum times ten, plus the digit, in MPAC, MPAC +1
 * (SHORTMP). An overflow of the low word ("OF MUST BE 5TH CHAR") goes
 * into the high one and the word is done (DECEND). */
static void dectobin(void)
{
    xch(indexed_erasable(ADDRESS_OF(VERBREG), fetch(ADDRESS_OF(INREL))));
    ts(ADDRESS_OF(MPAC)); /* "SUM X 2EXP-14 IN MPAC" */
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 1u);
    set_A(TEN); /* "10 X 2EXP-14" */
    tc_at(DECTOBIN + 6u);
    shortmp(); /* "10SUM X 2EXP-28 IN MPAC, MPAC+1" */
    xch(ADDRESS_OF(MPAC) + 1u);
    ad_at(ADDRESS_OF(CHAR));
    if (!ts(ADDRESS_OF(MPAC) + 1u)) {
        tc_at(DECTOBIN + 10u); /* "NO OF" */
        endnmtst();
        return;
    }
    ads(ADDRESS_OF(MPAC));
    tc_at(DECTOBIN + 12u);
    decend();
}

/* ENDNMTST -- LM p. 400, CM p. 317.
 * The register gets its new word. If DSPCOUNT is at the register's last
 * digit (CRITCON's word for INREL), the word is done (ENDNUM); else on to
 * the next digit (MORNUM). */
static void endnmtst(void)
{
    word inrel = fetch(ADDRESS_OF(INREL));

    ts(indexed_erasable(ADDRESS_OF(VERBREG), inrel));
    cs(ADDRESS_OF(DSPCOUNT));
    ad_at(indexed_by(ADDRESS_OF(CRITCON), inrel));
    if (A_is_zero()) { /* "-0, DSPCOUNT = CRITCON" */
        /* ENDNUM: a decimal word is made a fraction (DECEND). */
        set_A(THREE);
        mask_at(ADDRESS_OF(DECBRNCH));
        if (ccs_at(A_ADDRESS) == POSITIVE) {
            tc_at(ENDNUM + 3u);
            decend();
        } else
            endall();
        return;
    }
    tc_at(ENDNMTST + 7u); /* "- , DSPCOUNT G/ CRITCON" */
    mornum();
}

/* ENDALL -- LM p. 400, CM p. 317.
 * "BLOCK NUMIN BY PLACING DSPCOUNT NEGATIVELY". */
static void endall(void)
{
    cs(ADDRESS_OF(DSPCOUNT));
    tc_at(ENDALL + 1u);
    mornum_plus1();
}

/* DECEND -- LM pp. 400-401, CM pp. 317-318.
 * "IF INREL=0,1 (VBREG,NNREG) LEAVE WHOLE. IF INREL=2,3,4 (R1,R2,R3),
 * CONVERT TO FRAC: MULT SUM X 2EXP-28 IN MPAC, MPAC+1 BY 2EXP14/10EXP5,
 * GIVES (SUM/10EXP5)X2EXP-14 IN MPAC, +1, +2." (DMP, DECON.) Then the
 * sign: INDEX A; TC +0 with DECBRNCH's low bits goes to TC +DECSGN (1, +)
 * or to the word after it (2, -), which negates MPAC +1, +2 first. The
 * high word goes into the register, the low into its XREGLP word. */
static void decend(void)
{
    set_A(negate(ONE));
    ad_at(ADDRESS_OF(INREL));
    if (A_is_zero_or_minus()) {
        endall();
        return;
    }
    tc_at(DECEND + 4u);
    dmp(); /* ADRES DECON */
    set_A(THREE);
    mask_at(ADDRESS_OF(DECBRNCH));
    tc_at(DECEND + 9u);
    switch (a_word()) {
    case 1u: /* "+" */ tc_at(DECEND + 10u); break;
    case 2u: /* "- CASE" */
        dcs(ADDRESS_OF(MPAC) + 1u);
        dxch(ADDRESS_OF(MPAC) + 1u);
        break;
    default: /* never: DECEND runs for decimal words, 1 or 2 */ not_modelled();
    }

    /* +DECSGN */
    word inrel = fetch(ADDRESS_OF(INREL));
    xch(ADDRESS_OF(MPAC) + 2u);
    ts(indexed_erasable(ADDRESS_OF(XREGLP) - 2u, inrel));
    xch(ADDRESS_OF(MPAC) + 1u);
    ts(indexed_erasable(ADDRESS_OF(VERBREG), inrel));
    tc_at(PLUS_DECSGN + 6u);
    endall();
}

/* MORNUM +1: DSPCOUNT gets A; then the job ends ("TC ENDOFJOB"). */
static void mornum_plus1(void)
{
    ts(ADDRESS_OF(DSPCOUNT));
    tc(MORNUM + 2u, ENDOFJOB);
}

/* MORNUM -- LM p. 401, CM p. 318. "DECREMENT DSPCOUNT" (CCS DSPCOUNT; TS
 * DSPCOUNT), the next digit along. At +0 it stays. DSPCOUNT is never
 * negative here (NUM took no digit then); if it were, the AGC would run
 * CRITCON's first words, OCT 22 and OCT 20, as TC 22 and TC 20. */
static void mornum(void)
{
    switch (ccs_at(ADDRESS_OF(DSPCOUNT))) {
    case POSITIVE: mornum_plus1(); break;
    case IS_PLUS_ZERO: tc(MORNUM + 2u, ENDOFJOB); break;
    case NEGATIVE: tc(MORNUM + 3u, 022u); break;
    case IS_MINUS_ZERO: tc(MORNUM + 4u, 020u); break;
    }
}

/* GETINREL -- LM p. 401, CM p. 318.
 * "GETINREL GETS PROPER DATA REG REL ADDRESS FOR CURRENT C(DSPCOUNT) AND
 * PUTS IN INTO INREL. +0 VERBREG, 1 NOUNREG, 2 XREG, 3 YREG, 4 ZREG."
 * INRELTAB's fifteenth word, for the DSPCOUNT no digit has, is TC
 * CCSHOLE. */
static bool getinrel_returns(void)
{
    ca(indexed_by(ADDRESS_OF(INRELTAB), fetch(ADDRESS_OF(DSPCOUNT))));
    ts(ADDRESS_OF(INREL));
    tc_return(Q_ADDRESS);
    return true;
}

/* ---- VERB and NOUN ---- */

static void nvcom(void);

/* VERB -- LM p. 402, CM p. 319. A new verb: VERBREG +0, its two digits
 * blanked, and DSPCOUNT at its left one (VD1). */
void verb(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(VERBREG));
    set_A(VD1);
    nvcom();
}

/* NOUN -- LM p. 402, CM p. 319. A new noun, the same at ND1 ("OCT 21 (DEC
 * 17)"). */
void noun(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(NOUNREG));
    set_A(ND1);
    tc_at(NOUN + 3u);
    nvcom();
}

/* NVCOM -- LM p. 402, CM p. 319.
 * DECBRNCH +1, "SET FOR DEC V/N CODE": the verb and noun are keyed in
 * decimal. REQRET +0, "SET FOR ENTPAS0". "IF DSPALARM OCCURS BEFORE FIRST
 * ENTPAS0 OR NVSUB, ENTRET MUST ALREADY BE SET TO TC ENDOFJOB". */
static void nvcom(void)
{
    ts(ADDRESS_OF(DSPCOUNT));
    tc_at(NVCOM + 1u);
    if (!two_blank_returns()) return;
    set_A(ONE);
    ts(ADDRESS_OF(DECBRNCH));
    set_A(ZERO);
    ts(ADDRESS_OF(REQRET));
    set_A(ENDINST);
    ts(ADDRESS_OF(ENTRET));
    tc(NVCOM + 8u, ENDOFJOB);
}

/* ---- The sign keys ---- */

static bool signtest_returns(void);
static bool plus_on_returns(void);
static bool minus_on_returns(void);
static void bothsgn(void);

/* NEGSGN -- LM p. 402, CM p. 319. The - key: if a sign may go in here
 * (SIGNTEST), the minus sign on (-ON), and DECBRNCH +2 (BOTHSGN). */
void negsgn(void)
{
    tc_at(NEGSGN);
    if (!signtest_returns()) return;
    tc_at(NEGSGN + 1u);
    if (!minus_on_returns()) return;
    set_A(TWO);
    bothsgn();
}

/* POSGN -- LM p. 402, CM p. 319. The + key, the same with +ON and +1. */
void posgn(void)
{
    tc_at(POSGN);
    if (!signtest_returns()) return;
    tc_at(POSGN + 1u);
    if (!plus_on_returns()) return;
    set_A(ONE);
    tc_at(POSGN + 3u);
    bothsgn();
}

/* BOTHSGN -- LM p. 402, CM p. 319.
 * "SET DEC COMP BIT TO 1 (IN DECBRNCH)": "BIT 5 FOR R1. BIT 4 FOR R2. BIT
 * 3 FOR R3." (INDEX INREL; AD BIT7, the bit table counting down), with the
 * sign's 1 or 2 in the low bits. Then FIXCLPAS: "IF CLPASS IS + OR +0.
 * MAKE IT +0." */
static void bothsgn(void)
{
    ad_at(indexed_by(ADDRESS_OF(BIT7), fetch(ADDRESS_OF(INREL))));
    ads(ADDRESS_OF(DECBRNCH));
    clpass_plus_zero(FIXCLPAS);
    tc(FIXCLPAS + 4u, ENDOFJOB);
}

/* +ON, -ON and SGNCOM -- LM pp. 402-403, CM pp. 319-320.
 * The sign of the register being loaded on the DSKY: its two relay bits
 * (SGNTAB, by INREL) are the + and the - sign; 11DSPIN puts the one in
 * SGNOFF out (CODE +0) and the one in SGNON in (CODE BIT11). The caller's
 * return is in L. */
static bool sgncom_returns(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CODE));
    xch(ADDRESS_OF(SGNOFF));
    tc_at(SGNCOM + 3u);
    if (!eleven_dspin_returns()) return false;
    set_A(BIT11);
    ts(ADDRESS_OF(CODE));
    xch(ADDRESS_OF(SGNON));
    tc_at(SGNCOM + 7u);
    if (!eleven_dspin_returns()) return false;
    tc_return(L_ADDRESS);
    return true;
}

/* +ON: the register's - sign bit (SGNTAB, by INREL: SGNTAB -2 is R1's)
 * off, the + bit, one higher, on. */
static bool plus_on_returns(void)
{
    lxch(Q_ADDRESS);
    tc_at(PLUS_ON + 1u);
    getinrel_returns();
    ca(indexed_by(ADDRESS_OF(SGNTAB) - 2u, fetch(ADDRESS_OF(INREL))));
    ts(ADDRESS_OF(SGNOFF));
    ad(ONE);
    ts(ADDRESS_OF(SGNON));
    return sgncom_returns();
}

/* -ON: the - bit on, the + bit off. */
static bool minus_on_returns(void)
{
    lxch(Q_ADDRESS);
    tc_at(MINUS_ON + 1u);
    getinrel_returns();
    ca(indexed_by(ADDRESS_OF(SGNTAB) - 2u, fetch(ADDRESS_OF(INREL))));
    ts(ADDRESS_OF(SGNON));
    ad(ONE);
    ts(ADDRESS_OF(SGNOFF));
    tc_at(MINUS_ON + 7u);
    return sgncom_returns();
}

void plus_on(void)
{
    plus_on_returns();
}

void minus_on(void)
{
    minus_on_returns();
}

/* SIGNTEST -- LM p. 403, CM p. 320.
 * "ALLOWS +,- ONLY WHEN DSPCOUNT=R1D1, R2D1, OR R3D1. ALLOWS ONLY FIRST OF
 * CONSECUTIVE +/- CHARACTERS." "IF LOW2 BITS OF DECBRNCH NOT= 0, SIGN FOR
 * THIS WORD ALREADY IN. REJECT." SGNTST1 is each test: back to SIGNTEST
 * (TC Q) on no match, to SIGNTEST's caller (TC L) on a match, "SIGN
 * LEGAL". */
static bool signtest_returns(void)
{
    static const struct {
        word first_digit;
        word at;
    } registers[] = {{R1D1, SIGNTEST + 6u}, {R2D1, SIGNTEST + 8u}, {R3D1, SIGNTEST + 10u}};
    unsigned i;

    lxch(Q_ADDRESS);
    set_A(THREE);
    mask_at(ADDRESS_OF(DECBRNCH)); /* CCS A: MASK leaves 0 to 3 */
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc(SIGNTEST + 4u, ENDOFJOB);
        return false;
    }
    for (i = 0; i < 3u; ++i) {
        set_A(negate(registers[i].first_digit));
        tc_at(registers[i].at);
        ad_at(ADDRESS_OF(DSPCOUNT)); /* SGNTST1 */
        if (A_is_zero()) {           /* EXTEND; BZF +2: "MATCH FOUND" */
            tc_return(L_ADDRESS);
            return true;
        }
        tc_return(Q_ADDRESS); /* TC Q: on to the next */
    }
    tc(SIGNTEST + 11u, ENDOFJOB); /* "NO MATCH FOUND. SIGN ILLEGAL" */
    return false;
}

/* ---- CLEAR ---- */

static void clpashi(void);
static void clear1(void);
static bool clr5_returns(void);

/* CLEAR -- LM p. 404, CM p. 321.
 * "CLEAR BLANKS WHICH R1, R2, R3 IS CURRENT OR LAST TO BE DISPLAYED
 * (PERTINENT XREG, YREG, ZREG IS CLEARED). SUCCESSIVE CLEARS TAKE CARE OF
 * EACH RX L/ RC UNTIL R1 IS DONE. THEN NO FURTHER ACTION. THE SINGLE
 * COMPONENT LOAD VERBS ALLOW ONLY THE SINGLE RC THAT IS APPROPRIATE TO BE
 * CLEARED. CLPASS +0 PASS0, CAN BE BACKED UP; +NZ HIPASS, CAN BE BACKED
 * UP; -NZ PASS0, CANNOT BE BACKED UP." INREL is the register DSPCOUNT +1
 * is in ("DO NOT CHANGE DSPCOUNT BECAUSE MAY LATER FAIL LEGALTST. MUST
 * SET INREL, EVEN FOR HIPASS."); a higher pass goes back one register
 * (CLPASHI). */
void clear(void)
{
    switch (ccs_at(ADDRESS_OF(DSPCOUNT))) {
    case POSITIVE:
        ad(ONE);
        /* fall through */
    case IS_PLUS_ZERO:
        tc_at(CLEAR + 2u); /* TC +2 */
        break;
    case NEGATIVE: ad(ONE); break;
    case IS_MINUS_ZERO: break;
    }
    ca(indexed_by(ADDRESS_OF(INRELTAB), a_word()));
    ts(ADDRESS_OF(INREL));
    switch (ccs_at(ADDRESS_OF(CLPASS))) {
    case POSITIVE:
        tc_at(CLEAR + 8u);
        clpashi();
        return;
    case IS_PLUS_ZERO: tc_at(CLEAR + 9u); break; /* TC +2 */
    case NEGATIVE: tc_at(CLEAR + 10u); break;    /* TC +1 */
    case IS_MINUS_ZERO: break;
    }
    /* "IF CLPASS IS +0 OR -, IT IS PASS0" */
    ca(ADDRESS_OF(INREL));
    tc_at(CLEAR + 12u);
    if (!legaltst_returns()) return;
    tc_at(CLEAR + 13u);
    clear1();
}

/* CLPASHI -- LM p. 404, CM p. 321.
 * A higher pass: the register before (INREL less one), if legal; REQRET
 * "+3 TO - NUMBER, BACKS DATA REQUESTS"; the verb one less "AND
 * RE-DISPLAY" (UPDATVB in bank 41, whose return is CLPASHI +11). INREL
 * waits in MIXTEMP. CCS INREL's -0 and - words (the CAF and the ADS) never
 * run first: INREL is never negative. */
static void clpashi(void)
{
    switch (ccs_at(ADDRESS_OF(INREL))) {
    case POSITIVE:
        ts(ADDRESS_OF(INREL));
        /* fall through */
    case IS_PLUS_ZERO:
        tc_at(CLPASHI + 2u);
        if (!legaltst_returns()) return;
        /* fall through */
    case NEGATIVE:
        ca(ADDRESS_OF(DOUBLK) + 2u);
        /* fall through */
    case IS_MINUS_ZERO: ads(ADDRESS_OF(REQRET)); break;
    }
    ca(ADDRESS_OF(INREL));
    ts(ADDRESS_OF(MIXTEMP));    /* "TEMP STORAGE FOR INREL" */
    dim(ADDRESS_OF(VERBREG));   /* "DECREMENT VERB AND RE-DISPLAY" */
    tc(CLPASHI + 9u, BANKCALL); /* CADR UPDATVB */
}

/* CLPASHI +11, after UPDATVB: INREL back ("RESTORE INREL"), then CLEAR1. */
void clpashi_after_updatvb(void)
{
    ca(ADDRESS_OF(MIXTEMP));
    ts(ADDRESS_OF(INREL));
    clear1();
}

/* CLEAR1 -- LM p. 404, CM p. 321. The register blanked (CLR5), and CLPASS
 * one up, "ONLY IF CLPASS IS + OR +0. SET FOR HIGHER PASS." */
static void clear1(void)
{
    tc_at(CLEAR1);
    if (!clr5_returns()) return;
    incr(ADDRESS_OF(CLPASS));
    tc(CLEAR1 + 2u, ENDOFJOB);
}

/* LEGALTST -- LM p. 404, CM p. 321.
 * INREL (in A) must be a data register: "LEGAL INREL G/ 2" and "LEGAL
 * INREL=2" (2 - 2 is -0) back to the caller, "ILLEGAL INREL=0,1" to
 * ENDOFJOB. (+0 cannot come of adding NEG2.) */
static bool legaltst_returns(void)
{
    ad(NEG2);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_return(Q_ADDRESS); return true;
    case IS_PLUS_ZERO: tc(LEGALTST + 3u, CCSHOLE); return false;
    case NEGATIVE: tc(LEGALTST + 4u, ENDOFJOB); return false;
    case IS_MINUS_ZERO: break;
    }
    tc_return(Q_ADDRESS);
    return true;
}

/* ---- The blanking routines ---- */

/* 5BLANK -- LM pp. 404-405, CM pp. 321-322.
 * "5BLANK BLANKS 5 CHAR DISPLAY WORD IN R1, R2, OR R3. IT ALSO ZEROES XREG,
 * YREG, OR ZREG. PLACE ANY + DSPCOUNT NUMBER FOR PERTINENT RC INTO
 * DSPCOUNT. DSPCOUNT IS LEFT SET TO LEFT MOST DSP NUMB FOR RC JUST
 * BLANKED." The word before it, LEGALTST +6, sets DSPCOUNT from A first
 * ("NEEDED FOR BLANKSUB"). CLR5 "USES 5BLANK BUT AVOIDS ITS TC GETINREL".
 * The return is in L. */
static bool five_blank_returns(void)
{
    lxch(Q_ADDRESS);
    tc_at(FIVE_BLANK + 1u);
    getinrel_returns();
    return five_blank_plus2_returns();
}

/* 5BLANK +2: the register, its low word and CODE zeroed; DECBRNCH's sign
 * bit for the register and low two bits cleared ("ZERO PERTINENT DEC COMP
 * BIT. PROTECT OTHERS", BRNCHCON "ZERO LOW 2 BITS"); the register's odd
 * first character "BLANK ISOLATED CHAR SEPARATELY" (SINBLANK, DSPIN), then
 * its other four two by two (5BLANK1: DOUBLK, 2BLANK twice); DSPCOUNT at
 * its first digit. */
static bool five_blank_plus2_returns(void)
{
    word inrel = fetch(ADDRESS_OF(INREL));

    set_A(ZERO);
    ts(indexed_erasable(ADDRESS_OF(VERBREG), inrel)); /* "ZERO X, Y, Z, REG." */
    ts(indexed_erasable(ADDRESS_OF(XREGLP) - 2u, inrel));
    ts(ADDRESS_OF(CODE));
    cs(indexed_by(ADDRESS_OF(BIT7), inrel));
    mask_at(ADDRESS_OF(DECBRNCH));
    mask(BRNCHCON);
    ts(ADDRESS_OF(DECBRNCH));
    ca(indexed_by(ADDRESS_OF(SINBLANK) - 2u, inrel));
    ts(ADDRESS_OF(COUNT));
    tc_at(FIVE_BLANK + 16u);
    if (!dspin_returns()) return false;

    /* 5BLANK1 */
    inrel = fetch(ADDRESS_OF(INREL));
    ca(indexed_by(ADDRESS_OF(DOUBLK) - 2u, inrel));
    ts(ADDRESS_OF(DSPCOUNT));
    tc_at(FIVE_BLANK1 + 3u);
    if (!two_blank_returns()) return false;
    set_A(negate(TWO));
    ads(ADDRESS_OF(DSPCOUNT));
    tc_at(FIVE_BLANK1 + 6u);
    if (!two_blank_returns()) return false;
    ca(indexed_by(ADDRESS_OF(R1D1) - 2u, fetch(ADDRESS_OF(INREL))));
    ts(ADDRESS_OF(DSPCOUNT)); /* "SET DSPCOUNT TO LEFT MOST DSP NUMBER OF REG. JUST BLANKED" */
    tc_return(L_ADDRESS);
    return true;
}

/* CLR5 -- LM p. 404, CM p. 321. */
static bool clr5_returns(void)
{
    lxch(Q_ADDRESS);
    tc_at(CLR5 + 1u);
    return five_blank_plus2_returns();
}

void clr5(void)
{
    clr5_returns();
}

void five_blank_minus1(void)
{
    ts(ADDRESS_OF(DSPCOUNT));
    five_blank_returns();
}

void five_blank(void)
{
    five_blank_returns();
}

void five_blank_plus2(void)
{
    five_blank_plus2_returns();
}

/* 2BLANK -- LM p. 405, CM p. 322.
 * "2BLANK BLANKS TWO CHAR. PLACE DSP NUMBER OF LEFT CHAR OF THE PAIR INTO
 * DSPCOUNT. THIS NUMBER IS LEFT IN DSPCOUNT". The pair's DSPTAB word (half
 * DSPCOUNT, by TS SR) gets -BLANKCON, all blank; NOUT, the count of words
 * for T4RUPT to send, one more "IF OLD CONTENTS +". */
bool two_blank_returns(void)
{
    ca(ADDRESS_OF(DSPCOUNT));
    ts(ADDRESS_OF(SR));
    set_A(negate(BLANKCON));
    inhint();
    xch(indexed_erasable(ADDRESS_OF(DSPTAB), fetch(ADDRESS_OF(SR))));
    if (!A_is_zero_or_minus()) incr(ADDRESS_OF(NOUT)); /* EXTEND; BZMF +2 */
    relint();
    tc_return(Q_ADDRESS);
    return true;
}

void two_blank(void)
{
    two_blank_returns();
}
