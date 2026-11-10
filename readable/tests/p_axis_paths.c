/* SPDX-License-Identifier: MIT */
/* For the faithful port's P-AXIS_RCS_AUTOPILOT test on the readable tables
 * (make p-axis-cases): that test also checks that its cases take every
 * branch of the faithful routines, which count them (lm_pa_paths, built
 * with LM_PA_COUNT_PATHS). Readable routines count nothing, so each key of
 * the table the test runs here calls the faithful routine first, on copies
 * of the state, the io record and the channels, for the count only; then
 * the readable routine, whose result stands. The readable tables are linked
 * as readable_<name>, the faithful as faithful_<name>. */
#include "lm_p_axis.h"

extern const agc_routine_table readable_lm_p_axis_routines;
extern const agc_routine_table faithful_lm_p_axis_routines;

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
        (void)faithful_lm_p_axis_routines.entries[i].call(&copy, &s, device ? &io : 0);
    }
    return readable_lm_p_axis_routines.entries[i].call(program, state, device);
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
BOTH(9)
BOTH(10)
BOTH(11)
BOTH(12)
BOTH(13)
BOTH(14)
BOTH(15)
BOTH(16)
BOTH(17)
BOTH(18)
BOTH(19)
BOTH(20)
BOTH(21)
BOTH(22)
BOTH(23)
BOTH(24)
BOTH(25)
BOTH(26)
BOTH(27)

#define P16(z) (uint16_t)(016u * 1024u + ((z) - 02000u))

/* The table the test runs, with the faithful table's keys. */
static const agc_routine_entry entries[28] = {
    {P16(LM_PA_PAXIS), both_0},      {P16(LM_PA_CHEKBITS2), both_1},
    {P16(LM_PA_AFTERCHEK), both_2},  {P16(LM_PA_BACKP), both_3},
    {P16(LM_PA_TRAPS), both_4},      {P16(LM_PA_QRATE), both_5},
    {P16(LM_PA_RRATE), both_6},      {P16(LM_PA_AFTERACDT), both_7},
    {P16(LM_PA_SUPERJOB), both_8},   {P16(LM_PA_PRETIMCK), both_9},
    {P16(LM_PA_CHKVISFZ), both_10},  {P16(LM_PA_ABORTYZ2), both_11},
    {P16(LM_PA_TSNEXTP1), both_12},  {P16(LM_PA_TSNEXTP), both_13},
    {P16(LM_PA_DETENTCK), both_14},  {P16(LM_PA_ZATTRET), both_15},
    {P16(LM_PA_RATERROR), both_16},  {P16(LM_PA_PJETSLEC6), both_17},
    {P16(LM_PA_PJETSLEC1), both_18}, {P16(LM_PA_WRITEPRET), both_19},
    {P16(LM_PA_JTLSTRET), both_20},  {P16(LM_PA_JETSOFFRET), both_21},
    {P16(LM_PA_CALCPERR), both_22},  {P16(LM_PA_PURGENCY), both_23},
    {P16(LM_PA_PURGENCY4), both_24}, {P16(LM_PA_SPSRET), both_25},
    {P16(LM_PA_TJETRET), both_26},   {P16(LM_PA_SELALARM), both_27},
};
const agc_routine_table lm_p_axis_routines = {entries, 28};
