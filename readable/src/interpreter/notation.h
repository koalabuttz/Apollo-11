/* SPDX-License-Identifier: MIT */
/* Interpretive code in the listing's notation.
 *
 * Interpretive code is not machine code: it is data the interpreter
 * (src/interpreter/) reads word by word. A line of it in the listing
 * names up to two orders, and the lines after it their operands:
 *
 *     STATEUP   SET      BOF         ORDERS(SET, BOF),
 *                        VINTFLAG    SWITCH(SET, VINTFLAG),
 *                        SURFFLAG    SWITCH(BOF, SURFFLAG),
 *                        DOINT       ADDRESS(DOINT),
 *
 * These macros put each line into its word as the assembler (yaYUL) does,
 * so a table of them is the program the AGC ran, word for word, and reads
 * like the listing beside it. Each name an operand uses is the listing's,
 * as the interpreter reads it (I_<name>, src/agc/<m>_names.inc).
 *
 * The orders a table here uses, with their codes in yaYUL's table of
 * Block II interpretive orders. The switch orders (SET, CLEAR, BON, BOF,
 * CLRGO) share one code: which one it is, is in its operand. */
#ifndef READABLE_NOTATION_H
#define READABLE_NOTATION_H

#define ORDER_EXIT       0000u
#define ORDER_VLOAD      0001u
#define ORDER_VLOAD_STAR 0003u /* VLOAD* */
#define ORDER_VSL1       0004u
#define ORDER_VXSC       0015u
#define ORDER_SIN        0020u
#define ORDER_VSL2       0024u
#define ORDER_SL1        0024u /* (VSL2's code too, in yaYUL's table) */
#define ORDER_LXA_1      0026u /* LXA,1 */
#define ORDER_COS        0030u
#define ORDER_DLOAD      0031u
#define ORDER_SR1        0034u
#define ORDER_V_SC       0035u /* V/SC */
#define ORDER_SLOAD      0041u
#define ORDER_SLOAD_STAR 0043u /* SLOAD* */
#define ORDER_SXA_1      0046u /* SXA,1 */
#define ORDER_PDDL       0051u
#define ORDER_VSR3       0054u
#define ORDER_MXV        0055u
#define ORDER_PDVL       0061u
#define ORDER_DCOMP      0100u
#define ORDER_DDV        0105u
#define ORDER_UNIT       0120u
#define ORDER_VAD        0121u
#define ORDER_BZE        0122u
#define ORDER_VSU        0125u
#define ORDER_GOTO       0126u
#define ORDER_BVSU       0131u
#define ORDER_VXV        0141u
#define ORDER_RTB        0142u
#define ORDER_STADR      0150u
#define ORDER_DSU        0151u
#define ORDER_CALL       0152u
#define ORDER_STQ        0156u
#define ORDER_RVQ        0160u
#define ORDER_DAD        0161u
#define ORDER_SET        0162u
#define ORDER_CLEAR      0162u
#define ORDER_BON        0162u
#define ORDER_BOF        0162u
#define ORDER_CLRGO      0162u
#define ORDER_VSL8       0164u
#define ORDER_PUSH       0170u
#define ORDER_DMP        0171u
#define ORDER_SETPD      0175u

/* An order pair: each code plus one, the left order in bits 1-7 and the
 * right one in bits 8-14, complemented, so the word is negative (an
 * operand word is positive: that is how the interpreter tells them
 * apart). NEWOPS reads it with CCS, which takes the one back off. */
#define ORDERS(left, right) ((word)(~((ORDER_##left + 1u) | ((ORDER_##right + 1u) << 7)) & 077777u))
/* A line with one order. */
#define ORDER(left) ((word)(~(ORDER_##left + 1u) & 077777u))

/* A switch order's switch: the bit's number in the flag words, as the
 * flag word (number / 15, bits 9-14) and the bit in it (number % 15, bits
 * 1-4), with the switch order's own code in bits 5-8. */
#define SWITCH_SET   0060u
#define SWITCH_CLEAR 0260u
#define SWITCH_BON   0300u
#define SWITCH_BOF   0340u
#define SWITCH_CLRGO 0220u
#define SWITCH(order, flag)                                                                        \
    ((word)((((I_##flag) / 15u) << 8) | ((I_##flag) % 15u) | SWITCH_##order))

/* An address: an erasable word, a word in the work area or a place in
 * fixed memory (a branch's, a CALL's or a load's). */
#define ADDRESS(name) ((word)(I_##name))
/* The listing's "+3": an address n words past a label. */
#define ADDRESS_AT(name, n) ((word)(I_##name + (n)))
/* An address order's address (DLOAD's, ...) is written one more than the
 * address: DIRADRES reads it with CS and CCS, which leave it one less. The
 * assembler keeps 14 bits of it, so a fixed word's (its bank * 02000 plus
 * its address in the bank) loses the bank's high bits: the interpreter
 * reads it in the code's own bank. */
#define ADDRESS_PLUS_ONE(name)       ((word)((I_##name + 1u) & 037777u))
#define ADDRESS_PLUS_ONE_AT(name, n) ((word)((I_##name + (n) + 1u) & 037777u))

/* A store code, on a line of its own, with the address it stores to (plus
 * one, as above): STORE, STODL, STOVL or STCALL in bits 12-14. */
#define STORE(name)       ((word)(000000u | (I_##name + 1u)))
#define STORE_AT(name, n) ((word)(000000u | (I_##name + (n) + 1u)))
#define STODL(name)       ((word)(014000u | (I_##name + 1u)))
#define STODL_AT(name, n) ((word)(014000u | (I_##name + (n) + 1u)))
#define STOVL(name)       ((word)(024000u | (I_##name + 1u)))
#define STOVL_AT(name, n) ((word)(024000u | (I_##name + (n) + 1u)))
#define STCALL(name)      ((word)(034000u | (I_##name + 1u)))

/* The job's work area (its VAC area, or a NOVAC job's) word by word, as
 * the listing numbers it: "10D" is word 10 (decimal) from FIXLOC, written
 * one more as above. The push-down list starts at its word 0, so "0" (or
 * "0D") as an address is the list's first word. */
#define ADDRESS_PLUS_ONE_D(n) ((word)((n) + 1u))
#define STORE_D(n)            ((word)(000000u | ((n) + 1u)))
#define STODL_D(n)            ((word)(014000u | ((n) + 1u)))
#define STOVL_D(n)            ((word)(024000u | ((n) + 1u)))
#define STCALL_D(n)           ((word)(034000u | ((n) + 1u)))
/* "n,1": address n indexed by X1 (an order with a star, VLOAD* or
 * SLOAD*, takes its index register from it). It is written one more, as
 * any address. */
#define ADDRESS_PLUS_ONE_X1(n) ((word)((n) + 1u))
/* The store code after STADR, complemented: STADR stores what it finds on
 * the push-down list, and the code word after it reads negative, so the
 * interpreter takes it as STADR's (not as a new pair). */
#define AFTER_STADR(store) ((word)(~(store) & 077777u))

/* A place in a table of interpretive code: its F-bank, the address of its
 * first word in the bank (02000-03777), and its words. */
struct interpretive_code {
    unsigned bank;
    word first;
    unsigned count;
    const word *words;
};

#endif
