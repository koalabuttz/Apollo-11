/* SPDX-License-Identifier: MIT */
/* The readable DAPIDLER_PROGRAM (src/dapidler/), the LM's, as routines of
 * its flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_dapidler.c): lm_dapidler_routines, 9 keys (8 in
 * bank 16, ALTDSPLY in bank 20), and lm_dapidler_hot_routines, the 3 a DAP
 * pass reaches (CHEKBITS, CHEKMORE, ALTDSPLY).
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 5, 6, 12, 14, 30 and 31 from the program's hardware record and
 * the fixed words INDEX reads (LIMITS and DACLIMIT, each with the words
 * either side), and take the states the faithful routines take: a key's
 * own bank in FB, EBANK 6, and for CHEKBITS and CHEKMORE a caller's
 * return in Q in fixed memory. A run that stops at not_modelled() leaves
 * the state and the io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "lm_dapidler.h"
#include "../dapidler/dapidler.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK16          016u
#define BANK20          020u
#define EBANK6_REGISTER 03000u

static jmp_buf back;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

static uint16_t *channel(unsigned number)
{
    switch (number) {
    case CHAN5: return &hardware_now->channel5;
    case CHAN6: return &hardware_now->channel6;
    case CHAN12: return &hardware_now->channel12;
    case CHAN14: return &hardware_now->channel14;
    case CHAN30: return &hardware_now->channel30;
    case CHAN31: return &hardware_now->channel31;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

static void channel_out(unsigned number, word value)
{
    *channel(number) = value;
}

/* OVERSUB2's INDEX A; CS LIMITS (POSMAX, NEG1/2, BIT15) and NEEDLES's
 * INDEX A; CAF DACLIMIT (DEC -384, DEC 16000, DEC 384). */
static word fixed_memory(unsigned physical)
{
    unsigned dac = BANK20 * 02000u + (DACLIMIT - 02000u);

    if (physical == FF(K_LIMITS - 1u)) return POSMAX;
    if (physical == FF(K_LIMITS)) return NEG1_2;
    if (physical == FF(K_LIMITS + 1u)) return BIT15;
    if (physical == dac - 1u) return negate(KW_DACLIMITP1);
    if (physical == dac) return KW_DACLIMIT;
    if (physical == dac + 1u) return KW_DACLIMITP1;
    not_modelled();
    return 0;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, unsigned bank, bool returns, void (*routine)(void))
{
    mission_io *io = device, kept_io;
    agc_hardware kept_hardware;
    apollo_overlay_state before;

    if (!state || !program || !program->hardware || !bind_machine(program, state, io))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if ((FBANK & 076000u) != (word)(bank << 10) || EBANK != EBANK6_REGISTER)
        return EXEC_OUT_OF_SCOPE;
    if (returns && (Q < 02000u || Q > 07777u)) return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;

    before = *state;
    kept_io = *io;
    kept_hardware = *hardware_now;
    if (setjmp(back)) {
        *state = before;
        *io = kept_io;
        *hardware_now = kept_hardware;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name, bank, returns)                                                                   \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, bank, returns, name);                                   \
    }
KEY(chekbits, BANK16, true)
KEY(chekmore, BANK16, true)
KEY(dapidler, BANK16, false)
KEY(checkup, BANK16, false)
KEY(checkup_after_chekbits, BANK16, false)
KEY(startdap_after_zatteror, BANK16, false)
KEY(moreidle_after_qerrcalc, BANK16, false)
KEY(shutdown_after_calcperr, BANK16, false)
KEY(altdsply, BANK20, false)

#define B16(z) (uint16_t)(BANK16 * 1024u + ((z) - 02000u))
#define B20(z) (uint16_t)(BANK20 * 1024u + ((z) - 02000u))

/* By address, as the faithful tables have them. */
static const agc_routine_entry keys[] = {
    {B16(CHEKBITS), key_chekbits},
    {B16(CHEKMORE), key_chekmore},
    {B16(DAPIDLER), key_dapidler},
    {B16(CHECKUP), key_checkup},
    {B16(CHECKUP + 1u), key_checkup_after_chekbits},
    {B16(STARTDAP + 2u), key_startdap_after_zatteror},
    {B16(MOREIDLE + 2u), key_moreidle_after_qerrcalc},
    {B16(SHUTDOWN), key_shutdown_after_calcperr},
    {B20(ALTDSPLY), key_altdsply},
};

static const agc_routine_entry hot_keys[] = {
    {B16(CHEKBITS), key_chekbits},
    {B16(CHEKMORE), key_chekmore},
    {B20(ALTDSPLY), key_altdsply},
};

const agc_routine_table lm_dapidler_routines = {keys, COUNT(keys)};
const agc_routine_table lm_dapidler_hot_routines = {hot_keys, COUNT(hot_keys)};

#endif
