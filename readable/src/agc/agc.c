/* SPDX-License-Identifier: MIT */
/* The machine model of agc.h: memory addressing, register stores and the
 * channel instructions. Its effects are the AGC's, as the faithful port's
 * instruction helpers (c64-port/src/agc_image_ops.c) model them. */
#include <stdlib.h>
#include "agc.h"

struct agc_machine agc;

/* ---- Bank registers ---- */

/* BBANK holds FBANK's bank bits (076000) and EBANK's (03400, shifted down
 * to 07); a write to any of the three updates the other two. */
static void couple_banks(void)
{
    agc.memory[BBANK_ADDRESS] =
        (uint16_t)((agc.memory[FBANK_ADDRESS] & 076000u) | (agc.memory[EBANK_ADDRESS] >> 8));
}

void set_EBANK(word value)
{
    agc.memory[EBANK_ADDRESS] = (uint16_t)(value & 03400u);
    couple_banks();
}

void set_FBANK(word value)
{
    agc.memory[FBANK_ADDRESS] = (uint16_t)(value & 076000u);
    couple_banks();
}

void set_BBANK(word value)
{
    agc.memory[FBANK_ADDRESS] = (uint16_t)(value & 076000u);
    agc.memory[EBANK_ADDRESS] = (uint16_t)((value & 7u) << 8);
    couple_banks();
}

/* ---- Erasable memory ---- */

unsigned erasable_index(unsigned address)
{
    address &= 01777u;
    if (address < 01400u) return address;
    return ((agc.memory[EBANK_ADDRESS] >> 8) & 7u) * 0400u + (address - 01400u);
}

word fetch(unsigned address)
{
    return (word)(agc.memory[erasable_index(address)] & MINUS_ZERO);
}

void store(unsigned address, word value)
{
    unsigned at = erasable_index(address);
    value &= MINUS_ZERO;
    switch (at) {
    case EBANK_ADDRESS: set_EBANK(value); return;
    case FBANK_ADDRESS: set_FBANK(value); return;
    case BBANK_ADDRESS: set_BBANK(value); return;
    case 7u: return; /* ZERO: always +0 */
    case A_ADDRESS:
    case L_ADDRESS:
    case Q_ADDRESS: agc.memory[at] = value; return;
    /* The editing registers: CYR, SR, CYL and EDOP change what is stored. */
    case 020u: agc.memory[at] = (uint16_t)((value >> 1) | ((value & 1u) << 14)); return;
    case 021u: agc.memory[at] = (uint16_t)((value >> 1) | (value & SIGN_BIT)); return;
    case 022u: agc.memory[at] = (uint16_t)(((value << 1) | (value >> 14)) & MINUS_ZERO); return;
    case 023u: agc.memory[at] = (uint16_t)((value >> 7) & 0177u); return;
    default: agc.memory[at] = value; return;
    }
}

enum sign ccs(word value)
{
    set_A(diminished_abs(value));
    return sign_of(value);
}

enum sign ccs_register(unsigned address)
{
    reg16 value = agc.memory[address];
    reg16 magnitude = (value & 0100000u) ? (reg16)(~value & 0177777u) : value;
    word corrected = overflow_corrected(value);
    A = (reg16)(magnitude > 1u ? magnitude - 1u : 0u);
    correct_L();
    if (overflow_of(value) > 0) return POSITIVE;
    if (overflow_of(value) < 0) return NEGATIVE;
    return sign_of(corrected);
}

/* ---- Fixed memory ---- */

unsigned fixed_physical(word address)
{
    unsigned bank;
    address &= 07777u;
    if (address >= 06000u) return 3u * 02000u + (address - 06000u);
    if (address >= 04000u) return 2u * 02000u + (address - 04000u);
    bank = FBANK >> 10;
    if (bank >= 030u && (*agc.superbank & 0100u)) bank += 010u;
    return bank * 02000u + (address - 02000u);
}

/* ---- Multiplication ---- */

void mp(word value)
{
    word multiplicand = overflow_corrected(A), high, low;
    bool negative = ((multiplicand ^ value) & SIGN_BIT) != 0;
    uint32_t product;

    if (value == PLUS_ZERO || value == MINUS_ZERO) { /* a zero multiplier gives +0 */
        A = L = 0;
        return;
    }
    product = (uint32_t)((multiplicand & SIGN_BIT) ? negate(multiplicand) : multiplicand) *
              (uint32_t)((value & SIGN_BIT) ? negate(value) : value);
    high = (word)(product >> 14);
    low = (word)(product & 037777u);
    if (negative) {
        high = negate(high);
        low = negate(low);
    }
    A = sign_extend(high);
    L = sign_extend(low);
}

/* ---- Channel 11 ---- */

void wor_dsalmout(void)
{
    *agc.dsalmout = (uint16_t)((overflow_corrected(A) | *agc.dsalmout) & MINUS_ZERO);
    A = sign_extend(*agc.dsalmout);
}

void wand_dsalmout(void)
{
    *agc.dsalmout = (uint16_t)(overflow_corrected(A) & *agc.dsalmout & MINUS_ZERO);
    A = sign_extend(*agc.dsalmout);
}

/* ---- Other channels ---- */

/* The channel's word, as the binding connects it. */
static word channel_word(unsigned channel)
{
    if (!agc.channel_in) not_modelled();
    return (word)(agc.channel_in(channel) & MINUS_ZERO);
}

static void channel_write(unsigned channel, word value)
{
    if (!agc.channel_out) not_modelled();
    agc.channel_out(channel, value);
}

void read_channel(unsigned channel)
{
    A = sign_extend(channel_word(channel));
}

void write_channel(unsigned channel)
{
    channel_write(channel, overflow_corrected(A));
}

void rand_channel(unsigned channel)
{
    A = sign_extend((word)(overflow_corrected(A) & channel_word(channel)));
}

void wand_channel(unsigned channel)
{
    word value = (word)(overflow_corrected(A) & channel_word(channel));
    channel_write(channel, value);
    A = sign_extend(value);
}

void wor_channel(unsigned channel)
{
    word value = (word)(overflow_corrected(A) | channel_word(channel));
    channel_write(channel, value);
    A = sign_extend(value);
}

void not_modelled(void)
{
    if (agc.leave) agc.leave();
    abort(); /* a binding that runs such a routine always sets agc.leave */
}

/* ---- DTCB ---- */

void dtcb(word at)
{
    word address = overflow_corrected(A), bbcon = overflow_corrected(L);
    set_A((word)(at + 1u));
    set_L(BBANK);
    continue_at((word)(address & 07777u));
    set_BBANK(bbcon);
}

/* ---- Operands ---- */

/* The editing registers CYR, SR, CYL and EDOP are erasable words 020-023. */
static bool is_edited_word(unsigned at)
{
    return at >= 020u && at <= 023u;
}

/* An address naming an editing register, through the bank EBANK selects. */
static bool is_editing(unsigned address)
{
    return is_edited_word(erasable_index(address));
}

word read_rewriting(unsigned address)
{
    word value = read_word((word)address);
    if (address < 02000u && is_editing(address)) store(address, value);
    return value;
}

/* An operand as a register holds it: A, L or Q themselves (all 16 bits),
 * any other word sign-extended. */
static reg16 operand(unsigned address)
{
    return address < 3u ? agc.memory[address] : sign_extend(read_word((word)address));
}

/* A store of a register's value: into A, L or Q as it is, into memory
 * overflow-corrected. A store into fixed memory changes nothing. */
static void put(unsigned address, reg16 value)
{
    if (address < 3u)
        agc.memory[address] = value;
    else if (address < 02000u)
        store(address, overflow_corrected(value));
}

/* ---- Instructions on any operand ---- */

void ca(unsigned address)
{
    if (address >= 3u)
        A = sign_extend(read_rewriting(address));
    else
        A = agc.memory[address];
    correct_L();
}

void cs(unsigned address)
{
    if (address >= 3u)
        A = sign_extend(negate(read_rewriting(address)));
    else
        A = (reg16)~agc.memory[address];
    correct_L();
}

void ad_at(unsigned address)
{
    reg16 value = address >= 3u ? sign_extend(read_rewriting(address)) : agc.memory[address];
    A = add16(A, value);
    correct_L();
}

void su_at(unsigned address)
{
    if (address == A_ADDRESS)
        A = 0177777u;
    else
        A = add16(A, address >= 3u ? sign_extend(negate(read_rewriting(address)))
                                   : (reg16)~agc.memory[address]);
    correct_L();
}

void mask_at(unsigned address)
{
    if (address < 3u)
        A = (reg16)(A & agc.memory[address]);
    else
        mask(read_word((word)address));
    correct_L();
}

enum sign ccs_at(unsigned address)
{
    enum sign outcome;
    if (address < 3u) return ccs_register(address);
    outcome = ccs(read_word((word)address));
    if (address < 02000u && is_editing(address)) store(address, fetch(address));
    return outcome;
}

void incr(unsigned address)
{
    put(address, add16(1u, operand(address)));
    correct_L();
}

void aug(unsigned address)
{
    reg16 value = operand(address);
    put(address, add16((value & 0100000u) ? 0177776u : 1u, value));
    correct_L();
}

void dim(unsigned address)
{
    reg16 value = operand(address);
    if (value != 0u && value != 0177777u)
        put(address, add16((value & 0100000u) ? 1u : 0177776u, value));
    correct_L();
}

void ads(unsigned address)
{
    if (address == A_ADDRESS)
        A = add16(A, A);
    else {
        A = add16(A, operand(address));
        put(address, A);
    }
    correct_L();
}

/* DCA and DCS write an editing register they name back, after both reads. */
static void rewrite_named(unsigned address)
{
    if (is_edited_word(address)) store(address, fetch(address));
}

void dca(unsigned address)
{
    if (address == A_ADDRESS) return;
    L = sign_extend(overflow_corrected(operand(address + 1u)));
    A = operand(address);
    rewrite_named(address + 1u);
    rewrite_named(address);
}

void dcs(unsigned address)
{
    if (address == A_ADDRESS) { /* DCOM */
        A = (reg16)~A;
        L = sign_extend(overflow_corrected((reg16)~L));
        return;
    }
    L = sign_extend(overflow_corrected((reg16)~operand(address + 1u)));
    A = (reg16)~operand(address);
    rewrite_named(address + 1u);
    rewrite_named(address);
}

/* The carry of a low word's sum into the high word's: +1, -1 or none. */
static reg16 with_carry(reg16 high, reg16 low)
{
    switch (overflow_of(low)) {
    case 1: return add16(high, 1u);
    case -1: return add16(high, 0177776u);
    default: return high;
    }
}

void das(unsigned address)
{
    reg16 low, high;
    if (address == A_ADDRESS) { /* DDOUBL */
        low = add16(L, L);
        A = with_carry(add16(A, A), low);
        L = sign_extend(overflow_corrected(low));
        return;
    }
    low = add16(L, operand(address + 1u));
    high = with_carry(add16(A, operand(address)), low);
    A = overflow_of(high) ? overflow_unit(high) : 0u;
    L = 0;
    put(address + 1u, address + 1u < 3u ? sign_extend(overflow_corrected(low)) : low);
    put(address, high);
    correct_L();
}

void mp_at(unsigned address)
{
    mp(address < 3u ? overflow_corrected(agc.memory[address]) : read_word((word)address));
}

void dv_at(unsigned address)
{
    dv(address >= 3u ? sign_extend(read_word((word)address)) : Q);
}

/* ---- Division ---- */

/* The AGC's divide unit, as the faithful port models it (c64-port/src/
 * agc_hardware_divide.c, checked against yaAGC): fourteen quotient bits by
 * trial subtraction on a residual held negative, then the signs. Its
 * arithmetic also gives the AGC's results for a quotient that does not
 * fit (a dividend not smaller than the divisor). */
void dv(reg16 divisor)
{
    const reg16 RADIX = 040000u;
    reg16 high = A, low = sign_extend(overflow_corrected(L));
    bool dividend_negative = (high & 0100000u) != 0;
    bool divisor_negative = (divisor & 0100000u) != 0;
    /* The residual starts as the high word, held negative. */
    reg16 residual = dividend_negative ? high : (reg16)~high;
    reg16 magnitude_of_divisor = divisor_negative ? (reg16)~divisor : divisor;
    reg16 low_bits, quotient = 0, bit;
    bool low_negative;

    /* A zero high word takes the dividend's sign from L. */
    if (residual == 0177777u) dividend_negative = (low & 0100000u) != 0;
    if (dividend_negative) low = (reg16)~low;
    low = add16(low, RADIX);
    if (overflow_of(low) != 1) residual = add16(residual, 1u);
    low_negative = (low & 0100000u) != 0;
    low_bits = (reg16)(low & (RADIX - 1u));

    for (bit = RADIX / 2u; bit; bit /= 2u) {
        bool negative = (residual & 0100000u) != 0;
        reg16 magnitude = negative ? (reg16)(0177777u - residual) : residual;
        reg16 doubled =
            (reg16)(2u * (magnitude % RADIX) + (negative && (low_bits & bit) ? 1u : 0u));
        reg16 folded = negative ? (reg16)(0177777u - doubled) : doubled;
        reg16 trial = add16(folded, magnitude_of_divisor);
        bool accepted = (trial & 0100000u) != 0; /* the subtraction left it negative */
        quotient = (reg16)(2u * quotient + accepted);
        residual = accepted ? trial : folded;
    }
    A = (reg16)(quotient + (low_negative ? 0140000u : 0u));
    if (dividend_negative != divisor_negative) A = (reg16)~A;
    L = sign_extend(overflow_corrected(dividend_negative ? residual : (reg16)~residual));
}

/* ---- Exchanges ---- */

void xch(unsigned address)
{
    reg16 old = A;
    if (address != A_ADDRESS) {
        A = operand(address);
        put(address, old);
    }
    correct_L();
}

void lxch(unsigned address)
{
    reg16 old = L;
    if (address == 7u)
        L = 0;
    else if (address != L_ADDRESS) {
        L = operand(address);
        put(address, old);
    }
    correct_L();
}

void qxch(unsigned address)
{
    reg16 old = Q;
    if (address == 7u)
        Q = 0;
    else if (address != Q_ADDRESS) {
        Q = operand(address);
        put(address, old);
    }
    correct_L();
}

bool ts(unsigned address)
{
    reg16 value = A;
    int overflow = overflow_of(value);
    if (address != A_ADDRESS) {
        put(address, address < 3u ? value : sign_extend(overflow_corrected(value)));
        if (overflow) A = overflow_unit(value);
    }
    correct_L();
    return overflow != 0;
}

void dxch(unsigned address)
{
    reg16 old;
    if (address == A_ADDRESS) return; /* DXCH A is a no-op */
    old = operand(address + 1u);
    put(address + 1u, L);
    L = old;
    correct_L();
    old = operand(address);
    put(address, A);
    A = old;
    correct_L();
}
