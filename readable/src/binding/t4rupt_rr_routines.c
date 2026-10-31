/* SPDX-License-Identifier: MIT */
/* The readable radar checks and GPMATRIX (src/t4rupt/rr_monitor.c) as
 * routines of the LM's flight program, under the table name and keys of
 * the faithful port's (c64-port/src/lm_t4rupt_rr.c): lm_t4rupt_rr_routines.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 12, 30 and 33 from the program's hardware record, and take the
 * states the faithful routines take: in bank 06; back from SPSIN or SPCOS,
 * no overflow in A. */
#include <stddef.h>
#include "binding.h"
#include "../t4rupt/rr_monitor.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK06 014000u

static agc_hardware *hardware_now;

static uint16_t *channel(unsigned number)
{
    switch (number) {
    case CHAN12: return &hardware_now->channel12;
    case CHAN30: return &hardware_now->channel30;
    case CHAN33: return &hardware_now->channel33;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

/* Only WAND CHAN12 writes. */
static void channel_out(unsigned number, word value)
{
    *channel(number) = value;
}

static const struct {
    word at;
    void (*routine)(void);
    bool after_trig; /* SPSIN's or SPCOS's return */
} entries[] = {
    {RRAUTCHK, rrautchk, false},
    {RRAUTCHK + 024u, rrautchk_after_waitlist, false},
    {TRKFLCDU, trkflcdu, false},
    {RRGIMON, rrgimon, false},
    {RRGIMON + 022u, rrgimon_out_of_limits, false},
    {RRGIMON + 023u, rrgimon_within_limits, false},
    {MONREPOS + 011u, monrepos_after_waitlist, false},
    {GPMATRIX, gpmatrix, false},
    {GPMATRIX + 2u, gpmatrix_after_sin_mg, true},
    {GPMATRIX + 5u, gpmatrix_after_cos_mg, true},
    {GPMATRIX + 010u, gpmatrix_after_sin_og, true},
    {GPMATRIX + 017u, gpmatrix_after_cos_og, true},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;
    unsigned i;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if (FBANK != BANK06) return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    entry = Z;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != entry) continue;
        if (entries[i].after_trig && overflow_of(A) != 0) return EXEC_OUT_OF_SCOPE;
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 06, in the faithful table's order. */
#define B6(z) (uint16_t)(06u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B6(K_RRAUTCHK), run},        {B6(K_RRAUTCHK + 024u), run}, {B6(K_TRKFLCDU), run},
    {B6(K_RRGIMON), run},         {B6(K_RRGIMON + 022u), run},  {B6(K_RRGIMON + 023u), run},
    {B6(K_MONREPOS + 011u), run}, {B6(K_GPMATRIX), run},        {B6(K_GPMATRIX + 2u), run},
    {B6(K_GPMATRIX + 5u), run},   {B6(K_GPMATRIX + 010u), run}, {B6(K_GPMATRIX + 017u), run}};
const agc_routine_table lm_t4rupt_rr_routines = {routines, COUNT(routines)};

#endif
