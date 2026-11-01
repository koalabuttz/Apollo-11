/* SPDX-License-Identifier: MIT */
/* The readable GIMBAL_LOCK_AVOIDANCE (src/gimbal_lock/) as routines of
 * each mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_gimbal_lock.c, cm_gimbal_lock.c):
 * <m>_gimbal_lock_interpretive (the order pairs, all in bank 22) and
 * <m>_gimbal_lock_basic, which is empty: the file has no basic code.
 *
 * Plumbing only. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()); a pass that ends at the CALL
 * of DELCOMP or the SETGO to NEWANGL +1, out of the file, stops there, as
 * the faithful pair does.
 *
 * The states the faithful routines take: FB bank 22, Z at NEWOPS, BANKSET
 * the BBANK and INTBIT15 the bank's bit 15. ZERO +0, the bank registers
 * agreeing, NEWJOB +0 or a core set, FIXLOC one of the VAC areas, the
 * interrupt and superbank latches in range and every erasable word an
 * ordinary one. A run the readable code refuses part way leaves the state
 * as it was. */
#include <stddef.h>
#include "binding.h"
#include "../gimbal_lock/gimbal_lock.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* Keys and physical addresses in bank 22. */
#define BANK22 044000u
#define B22(z) (uint16_t)(022u * 1024u + ((z) - 02000u))

#if LUMINARY099
#define CODE NOGIMLOC_CODE
#else
#define CODE WCALC_CODE
#endif

/* The states the faithful routines take (its sane()). */
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
    if (Z != NEWOPS || FBANK != BANK22 || LOC < CODE.first || LOC >= CODE.first + CODE.count ||
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

#define K22(z) {B22(z), interpretive}
#if LUMINARY099
static const agc_routine_entry pairs[] = {
    K22(K_NOGIMLOC),        K22(K_NOGIMLOC + 02u),  K22(K_NOGIMLOC + 05u),
    K22(K_NOGIMLOC + 07u),  K22(K_NOGIMLOC + 012u), K22(K_NOGIMLOC + 014u),
    K22(K_NOGIMLOC + 017u), K22(K_NOGIMLOC + 021u), K22(K_NOGIMLOC + 022u),
};
const agc_routine_table lm_gimbal_lock_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_gimbal_lock_basic = {0, 0};
#else
static const agc_routine_entry pairs[] = {
    K22(K_WCALC),        K22(K_WCALC + 03u),  K22(K_WCALC + 05u),  K22(K_WCALC + 010u),
    K22(K_WCALC + 012u), K22(K_WCALC + 014u), K22(K_WCALC + 017u), K22(K_WCALC + 021u),
    K22(K_WCALC + 023u), K22(K_WCALC + 025u), K22(K_WCALC + 026u),
};
const agc_routine_table cm_gimbal_lock_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_gimbal_lock_basic = {0, 0};
#endif
