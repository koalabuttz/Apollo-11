/* SPDX-License-Identifier: MIT */
/* For the faithful port's DAPIDLER test on the readable tables (make
 * dapidler-cases): that test also checks that its cases take every branch
 * of the faithful routines, which count them (lm_di_paths, built with
 * LM_DI_COUNT_PATHS). Readable routines count nothing, so each key of the
 * tables the test runs here calls the faithful routine first, on copies of
 * the state, the io record and the channels, for the count only; then the
 * readable routine, whose result stands. The readable tables are linked
 * as readable_<name>, the faithful as faithful_<name>. */
#include "lm_dapidler.h"

extern const agc_routine_table readable_lm_dapidler_routines;
extern const agc_routine_table faithful_lm_dapidler_routines;

static enum exec_status both(unsigned i, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    agc_native_program copy;
    apollo_overlay_state s;
    lm_flight_io io;
    agc_hardware hw;

    if (program && state) {
        copy = *program;
        s = *state;
        if (program->hardware) {
            hw = *program->hardware;
            copy.hardware = &hw;
        }
        if (device) io.bias = *(const p66_bias_io *)device;
        (void)faithful_lm_dapidler_routines.entries[i].call(&copy, &s, device ? &io : 0);
    }
    return readable_lm_dapidler_routines.entries[i].call(program, state, device);
}

#define BOTH(i)                                                                                    \
    static enum exec_status both_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return both(i##u, program, state, device);                                                 \
    }
BOTH(0)
BOTH(1)
BOTH(2)
BOTH(3)
BOTH(4)
BOTH(5)
BOTH(6)
BOTH(7)
BOTH(8)

#define P16(z) (uint16_t)(016u * 1024u + ((z) - 02000u))
#define P20(z) (uint16_t)(020u * 1024u + ((z) - 02000u))

/* The table the test runs, with the faithful table's keys. */
static const agc_routine_entry entries[9] = {
    {P16(LM_DI_CHEKBITS), both_0}, {P16(LM_DI_CHEKMORE), both_1}, {P16(LM_DI_DAPIDLER), both_2},
    {P16(LM_DI_CHECKUP), both_3},  {P16(02042u), both_4},         {P16(02050u), both_5},
    {P16(02161u), both_6},         {P16(02163u), both_7},         {P20(LM_DI_ALTDSPLY), both_8},
};
const agc_routine_table lm_dapidler_routines = {entries, 9};
