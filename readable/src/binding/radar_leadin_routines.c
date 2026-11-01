/* SPDX-License-Identifier: MIT */
/* The readable radar sampling loop (src/radar_leadin/) as routines of the
 * LM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_radar_leadin_routines.c):
 * lm_radar_leadin_routines_basic, all in bank 25, and
 * lm_radar_leadin_routines_interpretive, which is empty (the file has no
 * interpretive code).
 *
 * Plumbing only. A routine runs from its key until control leaves the
 * file (WAITLIST, NOVAC, TASKOVER, BANKCALL, SWCALL, ENDOFJOB).
 *
 * The states the faithful routines take: FB bank 25, ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../radar_leadin/radar_leadin.h"
#include "../agc/names.h"

#if LUMINARY099

/* Bank 25, and a word's key in it. */
#define BANK25 052000u
#define B25(z) (uint16_t)(025u * 1024u + ((z) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* VARADAR reads RDRLOCS from the rope. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The states the faithful routines take (its basic_sane()). */
static bool sane(void)
{
    unsigned i;

    if (FBANK != BANK25 || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static const struct {
    word at;
    void (*routine)(void);
} entries[] = {
    {RADSAMP, radsamp},
    {RADSAMP + 6u, radsamp_after_waitlist},
    {RADSAMP + 012u, radsamp_after_novac},
    {DORSAMP, dorsamp},
    {DORSAMP + 1u, dorsamp_after_varadar},
    {DORSAMP + 3u, dorsamp_bad_return},
    {DORSAMP2, dorsamp2},
    {VARADAR, varadar},
};

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    unsigned i;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind_machine(program, state, io);
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    for (i = 0; i < COUNT(entries); ++i)
        if (entries[i].at == Z) break;
    if (i == COUNT(entries) || !sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entries[i].routine();
    return EXEC_OK;
}

/* ---- The keys, in the faithful table's order ---- */

#define B(z) {B25(z), basic}
static const agc_routine_entry basic_keys[] = {
    B(K_RADSAMP),      B(K_RADSAMP + 6u), B(K_RADSAMP + 012u), B(K_DORSAMP),
    B(K_DORSAMP + 1u), B(K_DORSAMP + 3u), B(K_DORSAMP2),       B(K_VARADAR),
};

const agc_routine_table lm_radar_leadin_routines_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_radar_leadin_routines_interpretive = {NULL, 0};

#endif
