/* SPDX-License-Identifier: MIT */
/* The readable LATITUDE_LONGITUDE_SUBROUTINES (src/lat_long/) as routines
 * of each mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_lat_long.c and cm_lat_long.c):
 * <m>_lat_long_interpretive, the order pairs, all in bank 13, and
 * <m>_lat_long_basic, which is empty: the file has no basic code. The
 * keys come from the faithful tables, in their order (lat_long_keys.inc
 * and cm_lat_long_keys.inc, which tools/p20_p25_keys.py writes from them).
 *
 * Plumbing only. An order pair is one pass of the readable interpreter
 * over the listing's words (interpret_listing()). The pairs take the
 * states the faithful ones take: FB bank 13, Z at NEWOPS, BANKSET the
 * BBANK and INTBIT15 the bank's bit 15; ZERO +0, NEWJOB +0 or a core set,
 * the bank registers agreeing, FIXLOC one of the VAC areas, the interrupt
 * and superbank latches in range and every erasable word an ordinary one.
 * A run the readable code refuses part way leaves the state as it was. */
#include <stddef.h>
#include "binding.h"
#include "../lat_long/lat_long.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define BANK13 026000u
#define CODE   LAT_LONG_CODE

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The states the faithful routines take (their sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    if (!bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || FBANK != BANK13 || LOC < CODE.first || LOC >= CODE.first + CODE.count ||
        BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, &CODE);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

#define BASIC(index, bank, address)
#define PAIR(index, bank, address) {KEY(bank, address), interpretive},
static const agc_routine_entry pairs[] = {
#if LUMINARY099
#include "lat_long_keys.inc"
#else
#include "cm_lat_long_keys.inc"
#endif
};
#undef BASIC
#undef PAIR

#if LUMINARY099
const agc_routine_table lm_lat_long_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_lat_long_basic = {0, 0};
#else
const agc_routine_table cm_lat_long_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_lat_long_basic = {0, 0};
#endif
