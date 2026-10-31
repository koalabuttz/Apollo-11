/* SPDX-License-Identifier: MIT */
/* The readable optics routines (src/t4rupt/optics.c) as routines of the
 * CM's flight program, under the table name and keys of the faithful
 * port's (c64-port/src/cm_t4rupt_optics.c): cm_t4rupt_optics_basic, with
 * the empty cm_t4rupt_optics_interpretive beside it (the optics have no
 * interpretive code).
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 12 and 14 from the mission's io record and 30 and 33 from the
 * program's hardware record, and take the states the faithful routines
 * take: in the routine's bank (06 or 10), the bank registers agreeing,
 * ZERO +0, the latches in range and every erasable word an ordinary one. */
#include <stddef.h>
#include "binding.h"
#include "../t4rupt/optics.h"
#include "../agc/names.h"

#if COMANCHE055

#define BANK06 014000u
#define BANK10 020000u

static agc_hardware *hardware_now;
static mission_io *io_now;

static uint16_t *channel(unsigned number)
{
    switch (number) {
    case CHAN12: return &io_now->channel12;
    case CHAN14: return &io_now->channel14;
    case CHAN30: return &hardware_now->channel30;
    case CHAN33: return &hardware_now->channel33;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

/* Only channels 12 and 14 are written (WAND, WOR). */
static void channel_out(unsigned number, word value)
{
    *channel(number) = value;
}

/* ---- The states the faithful routines take ---- */

static bool as_expected(word bank)
{
    unsigned i;

    if (FBANK != bank || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The entries ---- */

static const struct {
    word bank, at;
    void (*routine)(void);
} entries[] = {
    {BANK06, OPTMON, optmon},
    {BANK06, ZOPALARM + 2u, zopalarm_after_alarm},
    {BANK06, ZTOCSC + 4u, ztocsc_after_alarm},
    {BANK06, SETZOEND + 7u, setzoend_after_waitlist},
    {BANK06, ENDZOPT, endzopt},
    {BANK06, ENDZOPT + 6u, endzopt_after_vardelay},
    {BANK10, OPTDRIVE, optdrive},
    {BANK10, CONTDRVE + 016u, contdrve_after_alarm},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;
    unsigned i;

    if (!state || !program || !program->hardware || !device ||
        !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    io_now = device;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    entry = Z;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != entry || entries[i].bank != FBANK) continue;
        if (!as_expected(entries[i].bank)) return EXEC_OUT_OF_SCOPE;
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 06 and bank 10, in the faithful table's order. */
#define B6(z)  (uint16_t)(06u * 1024u + ((z) - 02000u))
#define B10(z) (uint16_t)(010u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B6(K_OPTMON), run},        {B6(K_ZOPALARM + 2u), run},   {B6(K_ZTOCSC + 4u), run},
    {B6(K_SETZOEND + 7u), run}, {B6(K_ENDZOPT), run},         {B6(K_ENDZOPT + 6u), run},
    {B10(K_OPTDRIVE), run},     {B10(K_CONTDRVE + 016u), run}};
const agc_routine_table cm_t4rupt_optics_basic = {routines, COUNT(routines)};
const agc_routine_table cm_t4rupt_optics_interpretive = {NULL, 0};

#endif
