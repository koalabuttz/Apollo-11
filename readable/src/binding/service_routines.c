/* SPDX-License-Identifier: MIT */
/* The readable service routines (src/service/) as routines of the flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/service_transfer.c): lm_service_routines and
 * cm_service_routines. The readable build links these in their place;
 * service_routines.c stays, as other faithful routines call it.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * fixed memory it reads, and take the states the faithful routines take. */
#include "binding.h"
#include "../service/service.h"
#include "../agc/names.h"

#define ZERO_REGISTER 7u
#define LAST_FLAG     14459u /* flag 15 * 963 + 14: FLAGWRD0 +963 is 01777 */

/* Keys: bank * 02000 + offset. DELAYJOB, TCGETCAD and WAKER are in bank
 * 0, the LM's UPENT2 and DOWNENT2 in bank 10. */
#define B(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))

#define WAKER_RETURN  (WAKER + 4u)   /* after TC JOBWAKE */
#define GENTRAN_PLUS1 (GENTRAN + 1u) /* the loop */

static const agc_native_program *program_now;

/* GENTRAN's data word, when it is in fixed memory: takes() reads it from
 * the rope before the routine runs. */
static struct {
    unsigned physical;
    word value;
} fixed_data_word;

/* The fixed words the routines read: GENTRAN's data, the bit table DEBIT
 * indexes (BIT15, BIT14 ... BIT1), and the ADRES words after a caller's
 * TC (the program's compiled operands). */
static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    if (physical == fixed_data_word.physical) return fixed_data_word.value;
    if (physical >= FF(BIT15_ADDRESS) && physical <= FF(BIT15_ADDRESS + 14u))
        return (word)(040000u >> (physical - FF(BIT15_ADDRESS)));
    agc_site_lookup(program_now->operands, program_now->operand_count, (uint16_t)physical, &value);
    return (word)(value & MINUS_ZERO);
}

/* ---- The states the faithful routines take ---- */

/* The bank registers agree, ZERO is +0, the superbank is one of the
 * eight, and interrupts are on or off. */
static bool banks_as_expected(void)
{
    return agc.memory[ZERO_REGISTER] == 0u && (EBANK & ~03400u) == 0u && (FBANK & ~076000u) == 0u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && (*agc.superbank & ~0160u) == 0u &&
           *agc.interrupts <= 1u;
}

/* Q holds the return address of a TC, in erasable or fixed memory. */
static bool q_is_a_return(void)
{
    return Q >= 0100u && Q <= 07775u;
}

/* An erasable address of ordinary data: not the registers, the counters or
 * the editing registers (physical 030 on). */
static bool ordinary(word address)
{
    return address < 02000u && erasable_index(address) >= 030u;
}

/* The word at address before the routine runs, or the caller's compiled
 * operand if address is in fixed memory. False if neither. */
static bool word_at(word address, word *value)
{
    agc_word operand;

    if (address < 02000u) {
        if (!ordinary(address)) return false;
        *value = fetch(address);
        return true;
    }
    if (address > 07777u || !agc_site_lookup(program_now->operands, program_now->operand_count,
                                             (uint16_t)fixed_physical(address), &operand))
        return false;
    *value = (word)(operand & MINUS_ZERO);
    return true;
}

/* The flag's name DEBIT reads at address, after its own stores: ITEMP1
 * holds 15 and ITEMP3 address +1 by then. */
static bool flag_name_at(word address, word *name)
{
    if (!word_at(address, name)) return false;
    if (address < 02000u && erasable_index(address) == ADDRESS_OF(ITEMP1)) *name = 15u;
    if (address < 02000u && erasable_index(address) == ADDRESS_OF(ITEMP3))
        *name = (word)(address + 1u);
    return true;
}

/* The flag word a flag's name points to is ordinary data. */
static bool flag_word_ordinary(word name)
{
    return name <= LAST_FLAG && ordinary((word)(ADDRESS_OF(FLAGWRD0) + name / 15u));
}

/* UPFLAG and DOWNFLAG: Q holds the address of the ADRES. COMFLAG returns
 * through ITEMP3, so the flag word may not be ITEMP3 itself (the faithful
 * routine returns past the ADRES even then; the AGC would go where the
 * new flag word points). */
static bool flag_call(void)
{
    word name;

    if (!q_is_a_return() || !flag_name_at(Q, &name) || !flag_word_ordinary(name)) return false;
    return erasable_index((word)(ADDRESS_OF(FLAGWRD0) + name / 15u)) != ADDRESS_OF(ITEMP3);
}

#if LUMINARY099
/* DEBIT, entered directly (the LM's INVFLAG): A holds the address of an
 * ADRES, Q a return. */
static bool debit_call(void)
{
    word name;

    return Q >= 0100u && Q <= 07777u && overflow_of(A) == 0 && a_word() <= 07777u &&
           overflow_of(add16(A, 1u)) == 0 && flag_name_at(a_word(), &name) &&
           flag_word_ordinary(name);
}

/* COMFLAG, entered directly: ITEMP1 names an ordinary flag word, and the
 * TC L at the end returns to an address (L after the LXCH, or after a TS
 * that overflowed and skipped it). */
static bool comflag_call(void)
{
    word flag = (word)(ADDRESS_OF(FLAGWRD0) + ITEMP1);
    reg16 back;

    if (ITEMP1 > 01777u - ADDRESS_OF(FLAGWRD0) || !ordinary(flag)) return false;
    if (overflow_of(A) != 0)
        back = L; /* all 16 bits: an overflowed L is no return address */
    else
        back = erasable_index(flag) == ADDRESS_OF(ITEMP3) ? a_word() : ITEMP3;
    return back >= 0100u && back <= 07777u;
}
#endif

/* GENTRAN: N-1 in A, at most 01777; the ADRES words L and M after the TC;
 * L + N-1 is data (ordinary erasable or fixed) and M + N-1 ordinary
 * erasable. A data word in fixed memory is read from the rope now. ITEMP1
 * holds N-1 when GENTRAN reads the ADRES words. */
static bool gentran_call(void)
{
    word n = a_word(), from, to;
    agc_word value;

    fixed_data_word.physical = 0;
    if (overflow_of(A) != 0 || n > 01777u || !q_is_a_return()) return false;
    if (!word_at(Q, &from) || !word_at((word)(Q + 1u), &to)) return false;
    if (Q < 02000u && erasable_index(Q) == ADDRESS_OF(ITEMP1)) from = n;
    if (Q + 1u < 02000u && erasable_index((word)(Q + 1u)) == ADDRESS_OF(ITEMP1)) to = n;
    if (overflow_of(add16(sign_extend(n), sign_extend(from))) != 0 ||
        overflow_of(add16(sign_extend(n), sign_extend(to))) != 0)
        return false;
    from = add(n, from);
    to = add(n, to);
    if (!ordinary(to) || from > 07777u) return false;
    if (from < 02000u) return ordinary(from);
    if (!program_now->fixed_data ||
        !program_now->fixed_data(program_now, (uint16_t)fixed_physical(from), &value))
        return false;
    fixed_data_word.physical = fixed_physical(from);
    fixed_data_word.value = (word)(value & MINUS_ZERO);
    return true;
}

static bool takes(word entry)
{
    if (!banks_as_expected()) return false;
    switch (entry) {
    case DELAYJOB: return FBANK == 0u && q_is_a_return();
    case TCGETCAD: return FBANK == 0u && RUPTREG1 <= DELAYNUM;
    case WAKER:
    case WAKER_RETURN: return FBANK == 0u;
    case UPFLAG:
    case DOWNFLAG: return flag_call();
    case GENTRAN:
    case GENTRAN_PLUS1: return gentran_call();
    case B5OFF: return true;
#if LUMINARY099
    case COMFLAG: return comflag_call();
    case DEBIT: return debit_call();
    case UPENT2:
    case DOWNENT2: return FBANK == 020000u && q_is_a_return();
#else
    case TRFAILOF:
    case TRFAILON:
    case UPENT2:
    case DOWNENT2: return q_is_a_return();
#endif
    default: return false;
    }
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    program_now = program;
    agc.fixed_memory = fixed_memory;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case DELAYJOB: delayjob(); break;
    case TCGETCAD: tcgetcad(); break;
    case WAKER: waker(); break;
    case WAKER_RETURN: waker_after_jobwake(); break;
    case UPFLAG: upflag(); break;
    case DOWNFLAG: downflag(); break;
    case GENTRAN: gentran(); break;
    case GENTRAN_PLUS1: gentran_plus1(); break;
    case B5OFF: b5off(); break;
    case UPENT2: upent2(); break;
    case DOWNENT2: downent2(); break;
#if LUMINARY099
    case COMFLAG: comflag(); break;
    case DEBIT: debit(); break;
#else
    case TRFAILOF: trfailof(); break;
    case TRFAILON: trfailon(); break;
#endif
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* In the faithful table's order (by key). */
#if LUMINARY099
static const agc_routine_entry routines[] = {
    {B(0u, DELAYJOB), run},     {B(0u, TCGETCAD), run}, {B(0u, WAKER), run},
    {B(0u, WAKER_RETURN), run}, {FF(UPFLAG), run},      {FF(COMFLAG), run},
    {FF(DOWNFLAG), run},        {FF(DEBIT), run},       {FF(GENTRAN), run},
    {FF(GENTRAN_PLUS1), run},   {FF(B5OFF), run},       {B(010u, UPENT2), run},
    {B(010u, DOWNENT2), run}};
const agc_routine_table lm_service_routines = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {
    {B(0u, DELAYJOB), run}, {B(0u, TCGETCAD), run}, {B(0u, WAKER), run}, {B(0u, WAKER_RETURN), run},
    {FF(UPFLAG), run},      {FF(DOWNFLAG), run},    {FF(GENTRAN), run},  {FF(GENTRAN_PLUS1), run},
    {FF(B5OFF), run},       {FF(TRFAILOF), run},    {FF(TRFAILON), run}, {FF(UPENT2), run},
    {FF(DOWNENT2), run}};
const agc_routine_table cm_service_routines = {routines, COUNT(routines)};
#endif
