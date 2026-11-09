/* SPDX-License-Identifier: MIT */
/* FIXED_FIXED_CONSTANT_POOL.agc -- LM pp. 1095-1099, CM pp. 1200-1204.
 * The faithful port reads these words from its compiled listing data
 * (c64-port/tools/fixed_data_generate.py).
 *
 * The constants every part of the program shares, in fixed-fixed memory
 * (banks 02 and 03, whatever FB holds), so that any bank can reach them
 * with a CAF. The readable routines name them by their labels (names.h,
 * from the assembled listing); here is the pool itself, as the listing
 * lays it out.
 *
 * Its words go up in value, from POSMAX to NEGONE (as one's complement
 * words read without their sign: 37777, the bits, the small numbers, the
 * priorities, then the negative words). Where a value of that order is
 * kept somewhere else, the listing comments out a line for it, naming the
 * file that has it ("INTERPRETER", "PINBALL", T4RUPT's RELTAB); those
 * lines stay here as comments, in their places. "THE FOLLOWING TABLE OF
 * 18 VALUES IS INDEXED": the bit table, BIT15 to BIT1, with NEG1/2 "TWO
 * LOCATIONS IN FRONT OF BIT14" (SPSIN reads NEG1/2 +2) and DPOSMAX before
 * POSMAX, a double precision 37777 37777; NEG0 and ZERO are the double
 * precision -0 and +0 ("DO NOT DESTROY THIS COMBINATION").
 *
 * The LM's pool has OCT33 and OCT35 (DEC27 and DEC29); the CM's has not.
 *
 * Each word is written as the listing has it: an OCT as the octal number,
 * a DEC integer as the decimal one, MINUS(n) for a negative number (its
 * one's complement), a DEC fraction as its word with the DEC in the
 * comment, VN(verb, noun) for a VN. fixed_fixed_constant() lays the words
 * out from DPOSMAX (block 02) and PRIO22 (block 03), where the listing
 * assembles them, and gives each one's value. */
#include <stddef.h>
#include "constant_pool.h"
#include "../agc/names.h"

/* A word of the pool: its label and value. */
struct pool_word {
    const char *label;
    word value;
};

#define WORD(label, value) {label, (word)(value)}
#define MINUS(n)           (077777u - (n))
#define VN(verb, noun)     ((verb) * 0200u + (noun))
#define COUNT(array)       (sizeof(array) / sizeof((array)[0]))

/* ---- Block 02, from DPOSMAX ---- */

static const struct pool_word block02[] = {
    /* "THE FOLLOWING TABLE OF 18 VALUES IS INDEXED. DO NOT INSERT OR REMOVE ANY QUANTITIES" */
    WORD("DPOSMAX", 037777), /* OCT 37777: "MUST PRECEDE POSMAX" */
    WORD("POSMAX", 037777),  /* OCT 37777 */
    /* LIMITS = NEG1/2 */
    WORD("NEG1/2", MINUS(020000)), /* OCT -20000: "USED BY SIN ROUTINE (MUST BE TWO" */
    /* "LOCATIONS IN FRONT OF BIT14)" */
    /* "BIT TABLE" */
    WORD("BIT15", 040000), /* OCT 40000 */
    WORD("BIT14", 020000), /* OCT 20000 */
    WORD("BIT13", 010000), /* OCT 10000 */
    WORD("BIT12", 04000),  /* OCT 04000 */
    WORD("BIT11", 02000),  /* OCT 02000 */
    WORD("BIT10", 01000),  /* OCT 01000 */
    WORD("BIT9", 0400),    /* OCT 00400 */
    WORD("BIT8", 0200),    /* OCT 00200 */
    WORD("BIT7", 0100),    /* OCT 00100 */
    WORD("BIT6", 040),     /* OCT 00040 */
    WORD("BIT5", 020),     /* OCT 00020 */
    WORD("BIT4", 010),     /* OCT 00010 */
    WORD("BIT3", 04),      /* OCT 00004 */
    WORD("BIT2", 02),      /* OCT 00002 */
    WORD("BIT1", 01),      /* OCT 00001 */
    /* "DO NOT DESTROY THIS COMBINATION, SINCE IT IS USED IN DOUBLE PRECISION INSTRUCTIONS." */
    WORD("NEG0", MINUS(0)), /* OCT -0: "MUST PRECEDE ZERO" */
    WORD("ZERO", 0),        /* OCT 0: "MUST FOLLOW NEG0" */
    /* (BIT1 OCT 00001) */
    /* (NO.WDS OCT 2: INTERPRETER) */
    /* (OCTAL3 OCT 3: INTERPRETER) */
    /* (R3D1 OCT 4: PINBALL) */
    WORD("FIVE", 05), /* OCT 5 */
    /* (REVCNT OCT 6: INTERPRETER) */
    WORD("SEVEN", 07), /* OCT 7 */
    /* (BIT4 OCT 00010) */
    /* (R2D1 OCT 11: PINBALL) */
    /* OCT11 = R2D1 */
    /* (BINCON DEC 10: PINBALL (OCTAL 12)) */
    WORD("ELEVEN", 11), /* DEC 11 */
    /* (OCT14 OCT 14: ALARM AND ABORT (FILLER)) */
    WORD("OCT15", 015), /* OCT 15 */
    /* (R1D1 OCT 16: PINBALL) */
    WORD("LOW4", 017), /* OCT 17 */
    /* ---- Page 1096 ---- */
    /* (BIT5 OCT 00020) */
    /* (ND1 OCT 21: PINBALL) */
    /* (VD1 OCT 23: PINBALL) */
    /* (OCT24 OCT 24: SERVICE ROUTINES) */
    /* (MD1 OCT 25: PINBALL) */
    WORD("BITS4&5", 030), /* OCT 30 */
/* (OCT31 OCT 31: SERVICE ROUTINES) */
#if LUMINARY099
    WORD("OCT33", 033), /* OCT 33 */
    /* DEC27 = OCT33 */
    WORD("OCT35", 035), /* OCT 35 */
/* DEC29 = OCT35 */
#endif
    WORD("CALLCODE", 032), /* OCT 00032 */
    /* (LOW5 OCT 37: PINBALL) */
    /* (33DEC DEC 33: PINBALL (OCTAL 41)) */
    /* (34DEC DEC 34: PINBALL (OCTAL 42)) */
    WORD("TBUILDFX", 37), /* DEC 37: "BUILDUP FOR CONVIENCE IN DAPTESTING" */
    WORD("TDECAYFX", 38), /* DEC 38: "CONVENIENCE FOR DAPTESTING" */
    /* (BIT6 OCT 00040) */
    WORD("OCT50", 050),    /* OCT 50 */
    WORD("DEC45", 45),     /* DEC 45 */
    WORD("SUPER011", 060), /* OCT 60: "BITS FOR SUPERBNK SETTING 011." */
    WORD(".5SEC", 50),     /* DEC 50 */
    /* (BIT7 OCT 00100) */
    /* SUPER100 = BIT7 */
    /* "(LAST 4K OF ROPE)" */
    WORD("SUPER101", 0120), /* OCT 120: "BITS FOR SUPERBNK SETTING 101" */
    /* (OCT121 OCT 121: SERVICE ROUTINES) */
    /* "(FIRST 8K OF ACM)" */
    WORD("SUPER110", 0140), /* OCT 140: "BITS FOR SUPERBNK SETTING 110." */
    /* "(LAST 8K OF ACM)" */
    WORD("1SEC", 100), /* DEC 100 */
    /* (LOW7 OCT 177: INTERPRETER) */
    /* (BIT8 OCT 00200) */
    /* (OT215 OCT 215: ALARM AND ABORT) */
    /* (8,5 OCT 00220: P20-P25 SUNDANCE) */
    WORD("2SECS", 200), /* DEC 200 */
    /* (LOW8 OCT 377: PINBALL) */
    /* (BIT9 OCT 00400) */
    WORD("GN/CCODE", 0401), /* OCT 00401: "SET S/C CONTROL SWITCH TO G/N" */
    WORD("3SECS", 300),     /* DEC 300 */
    WORD("4SECS", 400),     /* DEC 400 */
    WORD("LOW9", 0777),     /* OCT 777 */
    /* (BIT10 OCT 01000) */
    /* (5.5DEGS DEC .03056: P20-P25 SUNDANCE (OCTAL 00765)) */
    /* (OCT1103 OCT 1103: ALARM AND ABORT) */
    WORD("C5/2", 001124),      /* DEC .0363551: "(OCTAL 01124)" */
    WORD("V05N09", VN(5, 9)),  /* VN 0509: "(SAME AS OCTAL 1211)" */
    WORD("OCT1400", 01400),    /* OCT 01400 */
    WORD("V06N22", VN(6, 22)), /* VN 0622 */
    /* ---- Page 1097 ---- */
    /* (MID5 OCT 1740: PINBALL) */
    WORD("BITS2-10", 01776), /* OCT 1776 */
    WORD("LOW10", 01777),    /* OCT 1777 */
    /* (BIT11 OCT 02000) */
    /* (2K+3 OCT 2003: PINBALL) */
    WORD("LOW7+2K", 02177), /* OCT 2177: "OP CODE MASK + BANK 1 FBANK SETTING." */
    WORD("EBANK5", 02400),  /* OCT 02400 */
    WORD("PRIO3", 03000),   /* OCT 03000 */
    WORD("EBANK7", 03400),  /* OCT 03400 */
    /* (LOW11 OCT 3777: PINBALL) */
    /* (BIT12 OCT 04000) */
    /* (RELTAB OCT 04025: T4RUPT) */
    WORD("PRIO5", 05000), /* OCT 05000 */
    WORD("PRIO6", 06000), /* OCT 06000 */
    WORD("PRIO7", 07000), /* OCT 07000 */
    /* (BIT13 OCT 10000) */
    /* (OCT 10003: T4RUPT RELTAB +1D) */
    /* (13,7,2 OCT 10102: P20-P25 SUNDANCE) */
    WORD("PRIO11", 011000), /* OCT 11000 */
    /* (PRIO12 OCT 12000: BANKCALL) */
    WORD("PRIO13", 013000), /* OCT 13000 */
    WORD("PRIO14", 014000), /* OCT 14000 */
    /* (OCT 14031: T4RUPT RELTAB +2D) */
    WORD("PRIO15", 015000), /* OCT 15000 */
    WORD("PRIO16", 016000), /* OCT 16000 */
    /* (85DEGS DEC .45556: P20-P25 SUNDANCE (OCTAL 16450)) */
    WORD("PRIO17", 017000),   /* OCT 17000 */
    WORD("OCT17770", 017770), /* OCT 17770 */
    /* (BIT14 OCT 20000) */
    /* (OCT 20033: T4RUPT RELTAB +3D) */
    WORD("PRIO21", 021000), /* OCT 21000 */
};

/* ---- Block 03, from PRIO22 ---- */

static const struct pool_word block03[] = {
    WORD("PRIO22", 022000), /* OCT 22000: "SERVICE ROUTINES" */
    WORD("PRIO23", 023000), /* OCT 23000 */
    WORD("PRIO24", 024000), /* OCT 24000 */
    /* (5/8+1 OCT 24001: SINGLE PRECISION SUBROUTINES) */
    /* (OCT 24017: T4RUPT RELTAB +4D) */
    WORD("PRIO25", 025000), /* OCT 25000 */
    WORD("PRIO26", 026000), /* OCT 26000 */
    WORD("PRIO27", 027000), /* OCT 27000 */
    /* (CHRPRIO OCT 30000: PINBALL) */
    /* (OCT 30036: T4RUPT RELTAB +5D) */
    WORD("PRIO31", 031000), /* OCT 31000 */
    WORD("C1/2", 031103),   /* DEC .7853134: "(OCTAL 31103)" */
    WORD("PRIO32", 032000), /* OCT 32000 */
    WORD("PRIO33", 033000), /* OCT 33000 */
    WORD("PRIO34", 034000), /* OCT 34000 */
    /* (OCT 34034: T4RUPT RELTAB +6D) */
    /* ---- Page 1098 ---- */
    WORD("PRIO35", 035000),  /* OCT 35000 */
    WORD("PRIO36", 036000),  /* OCT 36000 */
    WORD("PRIO37", 037000),  /* OCT 37000 */
    WORD("63/64+1", 037401), /* OCT 37401 */
    /* (MID7 OCT 37600: PINBALL) */
    WORD("OCT37766", 037766), /* OCT 37766 */
    WORD("OCT37774", 037774), /* OCT 37774 */
    WORD("OCT37776", 037776), /* OCT 37776 */
    /* (DPOSMAX OCT 37777) */
    /* (BIT15 OCT 40000) */
    /* (OCT40001 OCT 40001: INTERPRETER ( CS 1 INSTRUCTION)) */
    WORD("DLOADCOD", 040014), /* OCT 40014 */
    WORD("DLOAD*", 040015),   /* OCT 40015 */
    /* (OCT 40023: T4RUPT RELTAB +7D) */
    WORD("BIT15+6", 040040),  /* OCT 40040 */
    WORD("OCT40200", 040200), /* OCT 40200 */
    /* (OCT 44035: T4RUPT RELTAB +8D) */
    /* (OCT 50037: T4RUPT RELTAB +9D) */
    /* (OCT 54000: T4RUPT RELTAB +10D) */
    WORD("-BIT14", 057777), /* OCT 57777 */
    /* (RELTAB11 OCT 60000: T4RUPT) */
    WORD("C3/2", 065552),          /* DEC -.3216147: "(OCTAL 65552)" */
    WORD("13,14,15", 070000),      /* OCT 70000 */
    WORD("-1/8", 073777),          /* OCT 73777 */
    WORD("HIGH4", 074000),         /* OCT 74000 */
    WORD("-ENDERAS", MINUS(2001)), /* DEC -2001: "(OCTAL 74056)" */
    /* (HI5 OCT 76000: PINBALL) */
    WORD("HIGH9", 077700), /* OCT 77700 */
    /* (-ENDVAC DEC -45: INTERPRETER (OCTAL 77722)) */
    /* (-OCT10 OCT -10: (OCT 77767)) */
    /* (NEG4 DEC -4: (OCTAL 77773)) */
    WORD("NEG3", MINUS(3)),   /* DEC -3 */
    WORD("NEG2", 077775),     /* OCT 77775 */
    WORD("NEGONE", MINUS(1)), /* DEC -1 */
};

/* "DEFINED BY EQUALS" (p. 1099; CM p. 1204): the names the listing gives
 * words of the pool or of other files. "IT WOULD BE TO THE USERS ADVANTAGE
 * TO OCCASIONALLY CHECK ANY OF THESE SYMBOLS IN ORDER TO PREVENT ANY
 * ACCIDENTAL DEFINITION CHANGES."
 *   MINUS1 = NEG1      NEG1 = NEGONE      ONE = BIT1         TWO = BIT2
 *   THREE = OCTAL3     LOW2 = THREE       FOUR = BIT3        SIX = REVCNT
 *   LOW3 = SEVEN       EIGHT = BIT4       NINE = R2D1        TEN = BINCON
 *   NOUTCON = ELEVEN   OCT23 = VD1        OCT25 = MD1        PRIO1 = BIT10
 *   EBANK3 = OCT1400   PRIO2 = BIT11      OCT120 = SUPER101  OCT140 = SUPER110
 *   2K = BIT11         EBANK4 = BIT11     PRIO4 = BIT12      EBANK6 = PRIO3
 *   QUARTER = BIT13    PRIO10 = BIT13     OCT10001 = CCSL    POS1/2 = HALF
 *   PRIO20 = BIT14     HALF = BIT14       PRIO30 = CHRPRIO   B12T14 = PRIO34
 *   BIT13-14 = PRIO30, "INTERPRETER USES IN PROCESSING STORECODE"
 *   OCT30002 = TLOAD +1                   NEGMAX = BIT15     VLOADCOD = BIT15
 *   VLOAD* = OCT40001  OCT60000 = RELTAB11                   BANKMASK = HI5 */

/* The pool's word at a fixed-fixed address, or NULL. */
static const struct pool_word *pool_word_at(word address)
{
    unsigned in02 = (unsigned)address - ADDRESS_OF(DPOSMAX);
    unsigned in03 = (unsigned)address - ADDRESS_OF(PRIO22);

    if (address >= ADDRESS_OF(DPOSMAX) && in02 < COUNT(block02)) return &block02[in02];
    if (address >= ADDRESS_OF(PRIO22) && in03 < COUNT(block03)) return &block03[in03];
    return NULL;
}

bool fixed_fixed_constant(word address, word *value)
{
    const struct pool_word *at = pool_word_at(address);

    if (!at) return false;
    *value = at->value;
    return true;
}

const char *fixed_fixed_constant_label(word address)
{
    const struct pool_word *at = pool_word_at(address);

    return at ? at->label : NULL;
}
