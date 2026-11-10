/* SPDX-License-Identifier: MIT */
/* For the faithful port's Q_R-AXIS_RCS_AUTOPILOT test on the readable tables
 * (make q-r-axis-cases): that test also checks that its cases take every
 * branch of the faithful routines, which count them (lm_qr_paths, built
 * with LM_QR_COUNT_PATHS). Readable routines count nothing, so each key of
 * the table the test runs here calls the faithful routine first, on copies
 * of the state, the io record and the channels, for the count only; then
 * the readable routine, whose result stands. The readable tables are linked
 * as readable_<name>, the faithful as faithful_<name>. */
#include "lm_q_r_axis.h"

extern const agc_routine_table readable_lm_q_r_axis_routines;
extern const agc_routine_table faithful_lm_q_r_axis_routines;

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
        (void)faithful_lm_q_r_axis_routines.entries[i].call(&copy, &s, device ? &io : 0);
    }
    return readable_lm_q_r_axis_routines.entries[i].call(program, state, device);
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
BOTH(28)
BOTH(29)

#define P17(z) (uint16_t)(017u * 1024u + ((z) - 02000u))

/* The table the test runs, with the faithful table's keys. */
static const agc_routine_entry entries[30] = {
    {P17(LM_QR_CALLQERR), both_0},     {P17(LM_QR_RCS), both_1},
    {P17(LM_QR_RCS1), both_2},         {P17(LM_QR_ALARMRET), both_3},
    {P17(LM_QR_QRCONTRL), both_4},     {P17(LM_QR_MINRET), both_5},
    {P17(LM_QR_MINRTN), both_6},       {P17(LM_QR_CHEKSTIK), both_7},
    {P17(LM_QR_ENTERUVRET), both_8},   {P17(LM_QR_BACKHAND), both_9},
    {P17(LM_QR_TRYGTS), both_10},      {P17(LM_QR_TIMEGMBLRET), both_11},
    {P17(LM_QR_QERRCALC), both_12},    {P17(LM_QR_STILLRCS), both_13},
    {P17(LM_QR_TJLAW3), both_14},      {P17(LM_QR_TJETRET), both_15},
    {P17(LM_QR_SPSRET), both_16},      {P17(LM_QR_WRITEPRET), both_17},
    {P17(LM_QR_NOROTATRET), both_18},  {P17(LM_QR_LOOPER), both_19},
    {P17(LM_QR_DOSKIP), both_20},      {P17(LM_QR_DOSKIP1), both_21},
    {P17(LM_QR_NOTMINRET), both_22},   {P17(LM_QR_XTRANS), both_23},
    {P17(LM_QR_WRITEURET), both_24},   {P17(LM_QR_WRITEVRET), both_25},
    {P17(LM_QR_JTLST), both_26},       {P17(LM_QR_ROTTOUV), both_27},
    {P17(LM_QR_SELALARMRET), both_28}, {P17(LM_QR_CLOSEOUT), both_29},
};
const agc_routine_table lm_q_r_axis_routines = {entries, 30};
