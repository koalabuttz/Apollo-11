/* SPDX-License-Identifier: MIT */
/* The readable STATEUP and R00's interpretive words (src/fresh_start/
 * stateup.c) as routines of the flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_stateup.c, cm_stateup.c):
 * lm_stateup_interpretive and cm_stateup_interpretive, with
 * lm_stateup_entries and cm_stateup_entries.
 *
 * Plumbing only. Each routine is one order pair, keyed by its address as
 * the interpreter fetches it (LOC at NEWOPS), and runs as one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * from the faithful routines' states: the pair's bank in BBANK and
 * BANKSET, no right-hand order pending (EDOP), a work area at a VAC area
 * or FAKEPRET with the push-down pointer in it, and at THISVINT's RVQ a
 * return address in fixed memory. The io record is the faithful
 * routines': a p66_bias_io (SUPERBNK and the interrupt state), which the
 * CM's interpreter keeps in its cm_native_io. */
#include <string.h>
#include "binding.h"
#include "../fresh_start/stateup.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_stateup.h"
#define THISVINT_RVQ 02066u /* THISVINT: CLEAR RVQ, RVQ returning to QPRET */
#else
#include "cm_stateup.h"
#define THISVINT_RVQ 02034u
#endif

#define ZERO_REGISTER 7u
#define FB_REGISTER   4u

/* Keys: bank 13 (STATEUP) and bank 04 (R00). */
#define K13(z) (uint16_t)(013u * 1024u + ((z) - 02000u))
#define K04(z) (uint16_t)(04u * 1024u + ((z) - 02000u))

static bool work_area(word fixloc)
{
    unsigned area;

    if (fixloc == FAKEPRET) return true; /* a NOVAC job's, MPAC -36D */
    for (area = 0401u; area <= 0661u; area += 054u)
        if (fixloc == area) return true;
    return false;
}

static bool takes(const apollo_overlay_state *state, const p66_bias_io *io,
                  const struct interpretive_code *code)
{
    const uint16_t *w = state->words;
    word fixloc = w[K_FIXLOC], loc = w[K_LOC];

    if (io->interrupt_enabled > 1u || (io->superbank & ~0160u) || w[ZERO_REGISTER] != 0u)
        return false;
    if (w[5] != NEWOPS || (w[FB_REGISTER] & 076000u) != (code->bank << 10) ||
        ((w[K_BANKSET] ^ w[FB_REGISTER]) & 076000u) || w[K_EDOP] != 0u)
        return false;
    if (!work_area(fixloc) || w[K_PUSHLOC] < fixloc || w[K_PUSHLOC] > fixloc + 042u) return false;
    if (loc == THISVINT_RVQ && !(w[(fixloc + 052u) & 03777u] & 074000u)) return false;
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, const struct interpretive_code *code)
{
    static const agc_native_program none;
    p66_bias_io *io = device;
#if COMANCHE055
    cm_native_io kept;
    enum exec_status status;
#endif

    if (!state || !io || !takes(state, io, code)) return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
    return interpret_listing(program ? program : &none, state, io, code);
#else
    memset(&kept, 0, sizeof kept);
    kept.superbank = io->superbank;
    kept.interrupt_enabled = io->interrupt_enabled;
    status = interpret_listing(program ? program : &none, state, &kept, code);
    io->superbank = kept.superbank;
    io->interrupt_enabled = kept.interrupt_enabled;
    return status;
#endif
}

static enum exec_status stateup(const agc_native_program *program, apollo_overlay_state *state,
                                void *device)
{
    return run(program, state, device, &STATEUP_CODE);
}

static enum exec_status r00(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    return run(program, state, device, &R00_CODE);
}

/* In the faithful table's order (by key): R00's two pairs in bank 04, then
 * STATEUP's in bank 13. */
#if LUMINARY099
static const agc_routine_entry routines[] = {
    {K04(02144), r00},     {K04(02146), r00},     {K13(02026), stateup}, {K13(02032), stateup},
    {K13(02036), stateup}, {K13(02041), stateup}, {K13(02045), stateup}, {K13(02047), stateup},
    {K13(02052), stateup}, {K13(02055), stateup}, {K13(02060), stateup}, {K13(02063), stateup},
    {K13(02066), stateup}};
const uint16_t lm_stateup_entries[LM_STATEUP_ROUTINES] = {02026u, 02032u, 02036u, 02041u, 02045u,
                                                          02047u, 02052u, 02055u, 02060u, 02063u,
                                                          02066u, 02144u, 02146u};
const agc_routine_table lm_stateup_interpretive = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {
    {K04(02131), r00},     {K04(02133), r00},     {K13(02000), stateup}, {K13(02004), stateup},
    {K13(02006), stateup}, {K13(02011), stateup}, {K13(02015), stateup}, {K13(02017), stateup},
    {K13(02022), stateup}, {K13(02026), stateup}, {K13(02031), stateup}, {K13(02034), stateup}};
const uint16_t cm_stateup_entries[CM_STATEUP_ROUTINES] = {
    02000u, 02004u, 02006u, 02011u, 02015u, 02017u, 02022u, 02026u, 02031u, 02034u, 02131u, 02133u};
const agc_routine_table cm_stateup_interpretive = {routines, COUNT(routines)};
#endif
