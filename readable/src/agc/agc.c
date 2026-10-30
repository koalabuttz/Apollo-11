/* SPDX-License-Identifier: MIT */
/* The machine model of agc.h: memory addressing, register stores and the
 * channel instructions. Its effects are the AGC's, as the faithful port's
 * instruction helpers (c64-port/src/agc_image_ops.c) model them. */
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

/* ---- DTCB ---- */

void dtcb(word at)
{
    word address = overflow_corrected(A), bbcon = overflow_corrected(L);
    set_A((word)(at + 1u));
    set_L(BBANK);
    continue_at((word)(address & 07777u));
    set_BBANK(bbcon);
}

/* ---- Exchanges ---- */

/* Each exchange works on registers too: an address below 3 is A, L or Q,
 * which keep all 16 bits. */
static reg16 register_or_word(unsigned address)
{
    return address < 3u ? agc.memory[address] : sign_extend(fetch(address));
}
static void put_register_or_word(unsigned address, reg16 value)
{
    if (address < 3u)
        agc.memory[address] = value;
    else
        store(address, overflow_corrected(value));
}

void xch(unsigned address)
{
    reg16 old = A;
    if (address == A_ADDRESS) return;
    A = register_or_word(address);
    put_register_or_word(address, old);
    correct_L();
}

void lxch(unsigned address)
{
    reg16 old = L;
    if (address == L_ADDRESS) return;
    if (address == 7u)
        L = 0;
    else {
        L = register_or_word(address);
        put_register_or_word(address, old);
    }
    correct_L();
}

void qxch(unsigned address)
{
    reg16 old = Q;
    if (address == Q_ADDRESS) return;
    if (address == 7u)
        Q = 0;
    else {
        Q = register_or_word(address);
        put_register_or_word(address, old);
    }
    correct_L();
}

bool ts(unsigned address)
{
    reg16 value = A;
    int overflow = overflow_of(value);
    if (address != A_ADDRESS)
        put_register_or_word(address,
                             address < 3u ? value : sign_extend(overflow_corrected(value)));
    if (overflow) A = overflow_unit(value);
    correct_L();
    return overflow != 0;
}

void das(unsigned address)
{
    reg16 low = add16(L, sign_extend(fetch(address + 1u)));
    reg16 high = add16(A, sign_extend(fetch(address)));
    if (overflow_of(low) > 0)
        high = add16(high, 1u);
    else if (overflow_of(low) < 0)
        high = add16(high, 0177776u);
    A = overflow_of(high) ? overflow_unit(high) : 0u;
    L = 0;
    store(address + 1u, overflow_corrected(low));
    store(address, overflow_corrected(high));
    correct_L();
}

void dxch(unsigned address)
{
    reg16 old;
    if (address == A_ADDRESS) return; /* DXCH A is a no-op */
    old = register_or_word(address + 1u);
    put_register_or_word(address + 1u, L);
    L = old;
    correct_L();
    old = register_or_word(address);
    put_register_or_word(address, A);
    A = old;
    correct_L();
}
