/* SPDX-License-Identifier: MIT */
/* The readable alarms and aborts (src/alarm/) as routines of the flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_alarm.c, cm_alarm.c). The readable build links these in
 * their place.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * fixed memory it reads (the alarm code after a caller's TC), and take the
 * states the faithful routines take. */
#include <stddef.h>
#include "binding.h"
#include "../alarm/alarm.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_alarm.h"
#else
#include "cm_alarm.h"
#endif

#define ZERO_REGISTER 7u

static const agc_native_program *program_now;

/* The fixed words the routines read are the alarm codes after the callers'
 * TCs (and a routine's own, OCT1103 or OCT217): the program's compiled
 * operands. takes() has found each before the routine runs. */
static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    agc_site_lookup(program_now->operands, program_now->operand_count, (uint16_t)physical, &value);
    return (word)(value & MINUS_ZERO);
}

/* ---- The states the faithful routines take ---- */

static bool machine_as_expected(void)
{
    return agc.memory[ZERO_REGISTER] == 0u && (*agc.superbank & ~0160u) == 0u &&
           *agc.interrupts <= 1u;
}

static bool no_overflow(reg16 value)
{
    return overflow_of(value) == 0;
}

/* INDEX Q; CA 0 can read the code at address: the caller's compiled
 * operand in fixed memory or, in the LM, an erasable word above the
 * registers. */
static bool code_readable_at(word address)
{
    agc_word value;

    if (address < 010u || address > 07777u) return false;
#if LUMINARY099
    if (address < 02000u) return true;
#else
    if (address < 02000u) return false;
#endif
    return agc_site_lookup(program_now->operands, program_now->operand_count,
                           (uint16_t)fixed_physical(address), &value) &&
           value <= MINUS_ZERO;
}

/* MULTFAIL's AD BIT15 cannot overflow: the alarm does not reach MULTFAIL
 * (one of the three FAILREG words is free) or its code is not negative. No
 * caller passes a negative code; the faithful routine leaves one that
 * overflows to the original program. */
static bool multfail_fits(word code)
{
    enum sign first = sign_of(FAILREG(0)), second = sign_of(FAILREG(1));
    bool all_taken = (first == POSITIVE || first == IS_MINUS_ZERO) &&
                     (second == POSITIVE || second == IS_MINUS_ZERO) && (FAILREG(2) & POSMAX) != 0u;
    return !all_taken || !(code & SIGN_BIT) || code == MINUS_ZERO;
}

/* INDEX Q; CA 0 can read the code, and it fits MULTFAIL. INDEX adds Q as a
 * word (overflow-corrected). The routine stores into ALMCADR before the
 * read (and into ALMCADR +1 too, after a DXCH): stored and stored_next,
 * which the read gets if Q points there. */
static bool code_at_q_fits(word stored, word stored_next, bool pair)
{
    word at = overflow_corrected(Q), code;

    if (!code_readable_at(at)) return false;
    if (at == K_ALMCADR)
        code = stored;
    else if (pair && at == K_ALMCADR + 1u)
        code = stored_next;
    else
        code = read_word(at);
    return multfail_fits(code);
}

/* The code is the word after the caller's TC, and the alarm returns past
 * it: MULTEXIT's INDEX A; TC 1 with A = Q is a TC (TC 1 + 07777 would be a
 * CCS), and LARMENT's TS ITEMP1 stores Q without an overflow. */
static bool returns_after_q(void)
{
    return no_overflow(Q) && Q <= 07776u;
}

static bool code_in_a_fits(void)
{
    return no_overflow(A) && multfail_fits(a_word());
}

/* WHIMPER's RESUME ends the interrupt the hardware record keeps. */
static bool can_resume(void)
{
    return agc.in_interrupt != NULL;
}

/* A routine's own code (OCT1103 after CCSHOLE's TC, OCT217 after
 * CURTAINS's) is a compiled operand too. */
static bool own_code_at(word address)
{
    return code_readable_at(address);
}

/* The return points, as the faithful tables key them. */
#define CURTAINS_RETURN ((word)(CURTAINS + 4u))
#define VARALARM_RETURN ((word)(VARALARM + 6u))
#define PRIOLARM_RETURN ((word)(PRIOLARM + 7u)) /* -2SEC +1 */
#if LUMINARY099
#define POODOO_RETURN   ((word)(ABORT2 + 5u)) /* OCT77770 +1 */
#define FLAGS_RETURN    ((word)(GOPOODOO + 3u))
#define V37KLEAN_RETURN ((word)(GOPOODOO + 9u))
#define MRKLEAN_RETURN  ((word)(GOPOODOO + 11u))
#define SUPERSW_RETURN  ((word)(STRTIDLE + 2u))
#else
#define BAILOUT_VAC5STOR_RETURN ((word)(BAILOUT + 5u))
#define POODOO_VAC5STOR_RETURN  ((word)(POODOO + 5u))
#define POODOO_RETURN           ((word)(POODOO + 9u)) /* OCT77770 +1 */
#define STATEFLG_RETURN         ((word)(POODOO + 15u))
#define REINTFLG_RETURN         ((word)(POODOO + 17u))
#define NODOFLAG_RETURN         ((word)(POODOO + 19u))
#define MRKLEAN_RETURN          ((word)(POODOO + 21u))
#define CCSHOLE_VAC5STOR_RETURN ((word)(CCSHOLE + 5u))
#endif
#define WHIMPER_MINUS1 ((word)(WHIMPER - 1u))

static bool takes(word entry)
{
    if (!machine_as_expected()) return false;
    switch (entry) {
    case ALARM: return returns_after_q() && code_at_q_fits(Q, 0, false);
    case ALARM2: return no_overflow(A) && returns_after_q() && code_at_q_fits(a_word(), 0, false);
    case CURTAINS: return no_overflow(Q) && own_code_at((word)(CURTAINS + 3u));
    case VARALARM: return no_overflow(Q) && code_in_a_fits();
    case PRIOLARM: return code_in_a_fits();
    case WHIMPER_MINUS1:
    case WHIMPER:
    case MRKLEAN_RETURN: return can_resume();
    case K_ALMCADR: return ALMCADR(0) >= 02000u && ALMCADR(0) <= 07777u;
#if LUMINARY099
    case BAILOUT:
    case POODOO: return no_overflow(Q) && code_at_q_fits(Q, 0, false);
    case ABORT2: return no_overflow(A) && code_at_q_fits(a_word(), 0, false);
    case BAILOUT1:
    case POODOO1: return code_at_q_fits(a_word(), overflow_corrected(L), true);
    case ALARM1: return returns_after_q() && code_at_q_fits(a_word(), overflow_corrected(L), true);
    case ALMNCADR: return returns_after_q() && code_at_q_fits(ALMCADR(0), 0, false);
    case CCSHOLE: return no_overflow(Q) && own_code_at((word)(CCSHOLE + 3u));
    case FLAGS: return no_overflow(Q) && Q >= 010u && Q <= 07777u;
#else
    case BAILOUT:
    case POODOO:
    case CCSHOLE: return no_overflow(Q);
    case BAILOUT_VAC5STOR_RETURN:
    case POODOO_VAC5STOR_RETURN: /* the code at ALMCADR */
        return code_readable_at(ALMCADR(0)) && multfail_fits(read_word(ALMCADR(0)));
    case POODOO_RETURN: /* WHIMPER -1 while average G is on */
        return (FLAGWRD7 & V37FLBIT) == 0u || can_resume();
#endif
    default: return true;
    }
}

/* ---- Running a routine ---- */

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
    case ALARM: alarm(); break;
    case ALARM2: alarm2(); break;
    case PRIOLARM: priolarm(); break;
    case PRIOLARM_RETURN: priolarm_after_alarm(); break;
    case CURTAINS: curtains(); break;
    case CURTAINS_RETURN: curtains_after_alarm(); break;
    case VARALARM: varalarm(); break;
    case VARALARM_RETURN: varalarm_after_alarm(); break;
    case K_ALMCADR: almcadr_as_instruction(); break;
    case BAILOUT: bailout(); break;
    case WHIMPER_MINUS1: whimper_minus1(); break;
    case WHIMPER: whimper(); break;
    case POODOO: poodoo(); break;
    case POODOO_RETURN: poodoo_after_alarm(); break;
    case CCSHOLE: ccshole(); break;
#if LUMINARY099
    case ABORT2: abort2(); break;
    case GOPOODOO: gopoodoo(); break;
    case FLAGS_RETURN: gopoodoo_after_flags(); break;
    case V37KLEAN_RETURN: gopoodoo_after_v37klean(); break;
    case MRKLEAN_RETURN: gopoodoo_after_mr_klean(); break;
    case SUPERSW_RETURN: strtidle_after_supersw(); break;
    case BAILOUT1: bailout1(); break;
    case POODOO1: poodoo1(); break;
    case ALARM1: alarm1(); break;
    case ALMNCADR: almncadr(); break;
    case FLAGS: flags(); break;
#else
    case BAILOUT_VAC5STOR_RETURN: bailout_after_vac5stor(); break;
    case POODOO_VAC5STOR_RETURN: poodoo_after_vac5stor(); break;
    case STATEFLG_RETURN: poodoo_after_stateflg(); break;
    case REINTFLG_RETURN: poodoo_after_reintflg(); break;
    case NODOFLAG_RETURN: poodoo_after_nodoflag(); break;
    case MRKLEAN_RETURN: poodoo_after_mr_klean(); break;
    case CCSHOLE_VAC5STOR_RETURN: ccshole_after_vac5stor(); break;
#endif
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* ---- The routine tables ---- */

#define B10(z) (uint16_t)(P_PRIOLARM + ((z) - PRIOLARM)) /* bank 10, by PRIOLARM's place */
#define ERASABLE_KEY(address) (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + (address))

#if LUMINARY099
static const agc_routine_entry routines[] = {
    {FF(ALARM), run},           {FF(ALARM2), run},
    {FF(BAILOUT), run},         {FF(WHIMPER_MINUS1), run},
    {FF(WHIMPER), run},         {FF(POODOO), run},
    {FF(ABORT2), run},          {FF(POODOO_RETURN), run},
    {FF(GOPOODOO), run},        {FF(FLAGS_RETURN), run},
    {FF(V37KLEAN_RETURN), run}, {FF(MRKLEAN_RETURN), run},
    {FF(SUPERSW_RETURN), run},  {FF(CCSHOLE), run},
    {FF(CURTAINS), run},        {FF(CURTAINS_RETURN), run},
    {FF(BAILOUT1), run},        {FF(POODOO1), run},
    {FF(ALARM1), run},          {FF(ALMNCADR), run},
    {FF(VARALARM), run},        {FF(VARALARM_RETURN), run},
    {B10(PRIOLARM), run},       {B10(PRIOLARM_RETURN), run},
    {(uint16_t)P_FLAGS, run},   {ERASABLE_KEY(K_ALMCADR), run}};
const agc_routine_table lm_alarm_routines = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {{FF(ALARM), run},
                                             {FF(ALARM2), run},
                                             {FF(POODOO), run},
                                             {FF(POODOO_VAC5STOR_RETURN), run},
                                             {FF(POODOO_RETURN), run},
                                             {FF(STATEFLG_RETURN), run},
                                             {FF(REINTFLG_RETURN), run},
                                             {FF(NODOFLAG_RETURN), run},
                                             {FF(MRKLEAN_RETURN), run},
                                             {FF(CCSHOLE), run},
                                             {FF(CCSHOLE_VAC5STOR_RETURN), run},
                                             {FF(CURTAINS), run},
                                             {FF(CURTAINS_RETURN), run},
                                             {FF(VARALARM), run},
                                             {FF(VARALARM_RETURN), run},
                                             {FF(BAILOUT), run},
                                             {FF(BAILOUT_VAC5STOR_RETURN), run},
                                             {FF(WHIMPER_MINUS1), run},
                                             {B10(PRIOLARM), run},
                                             {B10(PRIOLARM_RETURN), run},
                                             {ERASABLE_KEY(K_ALMCADR), run}};
const agc_routine_table cm_alarm_routines = {routines, COUNT(routines)};
#endif
