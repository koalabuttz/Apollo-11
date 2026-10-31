/* SPDX-License-Identifier: MIT */
/* The readable IMU compensation (src/imu_compensation/) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/imu_compensation.c): lm_imu_compensation_routines and
 * cm_imu_compensation_routines, bank 06.
 *
 * Plumbing only: point the machine at the program's state, and take the
 * states the faithful routines take: in bank 06, the bank registers
 * agreeing, ZERO +0, the latches in range and every erasable word an
 * ordinary one. The LM's NBDONLY +020 takes TEM1 as NBDONLY leaves it
 * before its IBNKCALL, +0 or BIT8; any other TEM1 is refused, as by the
 * faithful port. A step the readable code does not model leaves the
 * state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "../imu_compensation/imu_compensation.h"
#include "../agc/names.h"

#define BANK06 014000u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The routines read no fixed word and no channel. */
static word fixed_memory(unsigned physical)
{
    (void)physical;
    not_modelled();
    return 0;
}

static bool as_expected(void)
{
    unsigned i;

    if (FBANK != BANK06 || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
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
    {ONE_PIPA, one_pipa},
    {IRIG1 - 1u, irigcomp_after_novac},
    {ONE_GYRO, one_gyro},
    {LGCOMP + 4u, one_gyro_after_imupulse},
    {LGCOMP + 6u, one_gyro_after_imustall},
    {GCOMP1, gcomp1},
    {NBDONLY, nbdonly},
#if LUMINARY099
    {NBDONLY + 020u, nbdonly_after_pipasr},
#endif
    {LASTBIAS, lastbias},
    {LASTBIAS + 2u, lastbias_after_pipuse},
#if LUMINARY099
    {GCOMPZER, gcompzer},
#endif
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    apollo_overlay_state before;
    mission_io io_before;
    mission_io *io = device;
    unsigned i;

    if (!state || !device || !bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at != Z) continue;
        if (!as_expected()) return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
        if (entries[i].routine == nbdonly_after_pipasr && TEM1 != 0u && TEM1 != BIT8)
            return EXEC_OUT_OF_SCOPE;
#endif
        before = *state;
        io_before = *io;
        if (setjmp(back)) {
            *state = before;
            *io = io_before;
            return EXEC_OUT_OF_SCOPE;
        }
        entries[i].routine();
        return EXEC_OK;
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 06, in the faithful table's order. */
#define B6(z) (uint16_t)(06u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {
    {B6(K_1_PIPA), run},         {B6(K_IRIG1 - 1u), run},    {B6(K_1_GYRO), run},
    {B6(K_LGCOMP + 4u), run},    {B6(K_LGCOMP + 6u), run},   {B6(K_GCOMP1), run},
    {B6(K_NBDONLY), run},
#if LUMINARY099
    {B6(K_NBDONLY + 020u), run},
#endif
    {B6(K_LASTBIAS), run},       {B6(K_LASTBIAS + 2u), run},
#if LUMINARY099
    {B6(K_GCOMPZER), run},
#endif
};
#if LUMINARY099
const agc_routine_table lm_imu_compensation_routines = {routines, COUNT(routines)};
#else
const agc_routine_table cm_imu_compensation_routines = {routines, COUNT(routines)};
#endif
