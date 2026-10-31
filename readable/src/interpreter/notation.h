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

#define ORDER_EXIT  0000u
#define ORDER_DLOAD 0031u
#define ORDER_CALL  0152u
#define ORDER_RVQ   0160u
#define ORDER_SET   0162u
#define ORDER_CLEAR 0162u
#define ORDER_BON   0162u
#define ORDER_BOF   0162u
#define ORDER_CLRGO 0162u

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
 * address: DIRADRES reads it with CS and CCS, which leave it one less. */
#define ADDRESS_PLUS_ONE(name) ((word)(I_##name + 1u))

/* A store code, on a line of its own, with the address it stores to (plus
 * one, as above): STORE, STODL, STOVL or STCALL in bits 12-14. */
#define STCALL(name) ((word)(034000u | (I_##name + 1u)))

/* A place in a table of interpretive code: its F-bank, the address of its
 * first word in the bank (02000-03777), and its words. */
struct interpretive_code {
    unsigned bank;
    word first;
    unsigned count;
    const word *words;
};

#endif
