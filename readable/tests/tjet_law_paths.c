/* SPDX-License-Identifier: MIT */
/* For the faithful port's TJET_LAW test on the readable table (make
 * tjet-law-cases): that test also checks that its cases take every branch
 * of the faithful routine, which counts them (lm_tj_paths, built with
 * LM_TJ_COUNT_PATHS). The readable routine counts nothing, so the key the
 * test runs here calls the faithful routine first, on a copy of the state,
 * for the count only; then the readable routine, whose result stands. The
 * readable table is linked as readable_lm_tjet_law_routines, the faithful
 * as faithful_lm_tjet_law_routines. */
#include "lm_tjet_law.h"

extern const agc_routine_table readable_lm_tjet_law_routines;
extern const agc_routine_table faithful_lm_tjet_law_routines;

static enum exec_status both(const agc_native_program *program, apollo_overlay_state *state,
                             void *device)
{
    apollo_overlay_state s;

    if (state) {
        s = *state;
        (void)faithful_lm_tjet_law_routines.entries[0].call(program, &s, device);
    }
    return readable_lm_tjet_law_routines.entries[0].call(program, state, device);
}

static const agc_routine_entry entries[1] = {
    {(uint16_t)(017u * 1024u + (LM_TJ_TJETLAW - 02000u)), both}};
const agc_routine_table lm_tjet_law_routines = {entries, 1};
