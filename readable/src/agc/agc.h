/* SPDX-License-Identifier: MIT */
/* The machine the readable routines run on: the AGC's erasable memory, its
 * central registers and the few I/O channels the Executive and WAITLIST
 * touch, with the one's complement arithmetic of its adder.
 *
 * Every AGC name keeps its listing spelling (STYLE.md, Naming): A, L, Q and
 * Z are the central registers, NEWJOB or LST1(3) an erasable word, NOVAC
 * the fixed address of a routine and EXECBANK a constant in fixed memory.
 * This header defines the registers; erasable.h and the mission's labels
 * define the rest. */
#ifndef READABLE_AGC_H
#define READABLE_AGC_H

#include <stdbool.h>
#include <stdint.h>

/* An AGC word: 15 bits, one's complement, the sign in bit 15 of the
 * listing (bit 14 here, mask 040000). +0 is 000000 and -0 is 077777. */
typedef uint16_t word;

/* A, L and Q are 16 bits wide. Bit 16 of the listing (here 0100000) is the
 * sign; when bits 16 and 15 differ the register holds an overflow. A word
 * moves into a register sign-extended and out of it overflow-corrected. */
typedef uint16_t reg16;

#define PLUS_ZERO  ((word)000000)
#define MINUS_ZERO ((word)077777)
#define SIGN_BIT   ((word)040000)

/* The memory and channels of the program being run. The routine bindings
 * (src/binding/) point these at the flight program's state before each
 * call; nothing else changes them. */
struct agc_machine {
    uint16_t *memory;       /* 2048 erasable words; 0-7 are the central registers */
    uint16_t *superbank;    /* channel 7, SUPERBNK (bits 5-7) */
    uint16_t *interrupts;   /* 1 while interrupts are allowed (RELINT), 0 after INHINT */
    uint16_t *dsalmout;     /* channel 11, DSALMOUT (bit 2 is the COMP ACTY light) */
    uint16_t *in_interrupt; /* 1 while an interrupt routine (T3RUPT and its task) runs */
    /* Fixed memory (the rope), by physical address: bank * 02000 + offset. */
    word (*fixed_memory)(unsigned physical);
};
extern struct agc_machine agc;

/* ---- Central registers (erasable addresses 0-7) ---- */

#define A_ADDRESS     0u
#define L_ADDRESS     1u
#define Q_ADDRESS     2u
#define EBANK_ADDRESS 3u
#define FBANK_ADDRESS 4u
#define Z_ADDRESS     5u
#define BBANK_ADDRESS 6u

#define A (agc.memory[A_ADDRESS])
#define L (agc.memory[L_ADDRESS])
#define Q (agc.memory[Q_ADDRESS])
#define Z (agc.memory[Z_ADDRESS])

/* The bank registers are read as words; a write goes through set_*, which
 * keeps BBANK (both banks in one word) in step with EBANK and FBANK. */
#define EBANK ((word)agc.memory[EBANK_ADDRESS])
#define FBANK ((word)agc.memory[FBANK_ADDRESS])
#define BBANK ((word)agc.memory[BBANK_ADDRESS])
void set_EBANK(word value);
void set_FBANK(word value);
void set_BBANK(word value);

/* ---- One's complement arithmetic ---- */

static inline word negate(word value)
{
    return (word)(~value & MINUS_ZERO);
}

/* The 15-bit sum, with the end-around carry of the AGC's adder. */
static inline word add(word a, word b)
{
    uint32_t sum = (uint32_t)a + b;
    return (word)((sum & MINUS_ZERO) + (sum >> 15));
}

/* A word as a register holds it, and the word a register stores. */
static inline reg16 sign_extend(word value)
{
    return (reg16)((value & MINUS_ZERO) | ((value & SIGN_BIT) << 1));
}
static inline word overflow_corrected(reg16 value)
{
    return (word)((value & 037777u) | ((value >> 1) & SIGN_BIT));
}

/* The 16-bit sum in a register, where an overflow shows. */
static inline reg16 add16(reg16 a, reg16 b)
{
    uint32_t sum = (uint32_t)a + b;
    if (sum & 0200000u) sum = (sum + 1u) & 0177777u;
    return (reg16)sum;
}

/* +1 for a positive overflow, -1 for a negative one, else 0. */
static inline int overflow_of(reg16 value)
{
    switch (value & 0140000u) {
    case 040000u: return 1;
    case 0100000u: return -1;
    default: return 0;
    }
}

/* What TS leaves in A after storing an overflowed accumulator: +1 or -1. */
static inline reg16 overflow_unit(reg16 value)
{
    return overflow_of(value) > 0 ? (reg16)1u : (reg16)0177776u;
}

/* The four outcomes of CCS (count, compare and skip), in the order of the
 * four places it can continue. */
enum sign { POSITIVE, IS_PLUS_ZERO, NEGATIVE, IS_MINUS_ZERO };

static inline enum sign sign_of(word value)
{
    if (value == PLUS_ZERO) return IS_PLUS_ZERO;
    if (value == MINUS_ZERO) return IS_MINUS_ZERO;
    return (value & SIGN_BIT) ? NEGATIVE : POSITIVE;
}

/* |value| - 1, or +0 for either zero: what CCS leaves in A. */
static inline word diminished_abs(word value)
{
    word magnitude = (value & SIGN_BIT) ? negate(value) : value;
    return magnitude > 1u ? (word)(magnitude - 1u) : PLUS_ZERO;
}

/* CCS on a word: A gets its diminished absolute value. */
enum sign ccs(word value);
/* CCS A or CCS L, on the register's 16 bits: an overflow counts as its sign. */
enum sign ccs_register(unsigned address);

/* A as a word (what TS would store from it). */
static inline word a_word(void)
{
    return overflow_corrected(A);
}

/* Every AGC instruction ends with L sign-corrected (its 16th bit a copy of
 * its 15th); correct_L() does that after a step that wrote L raw. */
static inline void correct_L(void)
{
    L = sign_extend(overflow_corrected(L));
}

/* Load A with a word, as CA and CAF do. */
static inline void set_A(word value)
{
    A = sign_extend(value);
    correct_L();
}

static inline void set_L(word value)
{
    L = sign_extend(value);
}

/* ---- Erasable memory ---- */

/* The physical word an erasable address names: 0-01377 directly, 01400-01777
 * in the switched bank EBANK selects. */
unsigned erasable_index(unsigned address);

/* The word at an erasable address, and a store into it with the side effects
 * of a store into a register (bank coupling, the editing registers 020-023). */
word fetch(unsigned address);
void store(unsigned address, word value);

/* The erasable word itself, for named variables (erasable.h). */
#define ERASABLE(address) (agc.memory[erasable_index(address)])

/* DCA: A,L get the double word at address (the high word, then the low). */
void dca(unsigned address);

/* XCH, LXCH and DXCH: A, L, or A and L as a pair (the high word at the
 * address, the low word after it), exchanged with memory. */
void xch(unsigned address);
void lxch(unsigned address);
void dxch(unsigned address);
void qxch(unsigned address);

/* TS: store A, overflow-corrected. On an overflow A becomes +1 or -1 and the
 * next instruction is skipped: the result says whether it was. */
bool ts(unsigned address);

/* DAS: add A,L into the double word at address (A gets the overflow, L +0). */
void das(unsigned address);

/* INDEX k: an address plus C(k), in one's complement (-0 indexes nothing). */
static inline unsigned indexed(unsigned address, word index)
{
    return add((word)address, index);
}

/* ---- Fixed memory ---- */

/* The physical word a fixed address names: 04000-07777 are banks 2 and 3
 * whatever the bank registers hold, 02000-03777 the bank FBANK selects. In
 * banks 030-037 a set SUPERBNK bit 7 selects banks 040-043 instead. */
unsigned fixed_physical(word address);

/* The word at a fixed address, under the current FBANK and SUPERBNK. */
static inline word fetch_fixed(word address)
{
    return agc.fixed_memory(fixed_physical(address));
}

/* The word at any address, erasable (below 02000) or fixed: what INDEX Q;
 * CA 0 reads, the word after a TC, wherever the TC was. */
static inline word read_word(word address)
{
    return address < 02000u ? fetch(address) : fetch_fixed(address);
}

/* DOUBLE: A + A, in the 16-bit accumulator. */
static inline void double_A(void)
{
    A = add16(A, A);
    correct_L();
}

/* EXTEND; MP: A,L get the double-precision product of A (as a word) and
 * value: the high 14 bits and sign in A, the low 14 and sign in L. */
void mp(word value);

/* BZF's test: A is +0 or -0. */
static inline bool A_is_zero(void)
{
    return A == 0u || A == 0177777u;
}

/* MASK: A and'ed with a word (sign-extended to A's 16 bits). */
static inline void mask(word value)
{
    A = (reg16)(A & sign_extend(value));
}

/* ---- I/O channels and interrupts ---- */

static inline void inhint(void)
{
    *agc.interrupts = 0;
}
static inline void relint(void)
{
    *agc.interrupts = 1;
}

/* ROR SUPERBNK: A or'ed with channel 7. WRITE SUPERBNK: channel 7 from A. */
static inline void ror_superbnk(void)
{
    A = (reg16)(A | *agc.superbank);
}
static inline void write_superbnk(void)
{
    *agc.superbank = (uint16_t)(overflow_corrected(A) & 0160u);
}

/* WOR / WAND DSALMOUT (channel 11), leaving the channel in A. */
void wor_dsalmout(void);
void wand_dsalmout(void);

/* ---- Leaving a routine ---- */

/* TC at fixed address at: Q gets the address after it (the return). */
static inline void tc_at(word at)
{
    Q = (word)((at + 1u) & MINUS_ZERO);
}

/* TC to an erasable word that holds an address: the AGC runs the word as
 * the instruction TC <address>. So Z gets the address, and Q the address
 * after the word. RETURN (TC Q) is the case of register Q itself: Z gets
 * the return address and Q becomes 3. */
static inline void tc_through(unsigned address)
{
    word to = (word)agc.memory[erasable_index(address)];
    Q = (word)(address + 1u);
    Z = to;
    correct_L();
}

/* Continue at a fixed address. */
static inline void continue_at(word address)
{
    Z = address;
}

/* DTCB (DXCH Z) at address at: the 2CADR in A,L becomes Z and BBANK, and A,L
 * get the address after the DTCB and the old BBANK. */
void dtcb(word at);

#endif
