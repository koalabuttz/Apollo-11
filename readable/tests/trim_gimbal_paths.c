/* SPDX-License-Identifier: MIT */
/* For the faithful port's TRIM_GIMBAL_CONTROL_SYSTEM test on the readable
 * table (make trim-gimbal-cases): that test also checks that its cases
 * take every reachable branch of the faithful routines, which count them
 * (lm_tg_paths, built with LM_TG_COUNT_PATHS). The readable routines count
 * nothing, so each key the test runs here calls the faithful routine
 * first, on copies of the state, the io record and the hardware record,
 * for the count only; then the readable routine, whose result stands. The
 * readable table is linked as readable_lm_trim_gimbal_routines, the
 * faithful as faithful_lm_trim_gimbal_routines. */
#include "lm_trim_gimbal.h"

extern const agc_routine_table readable_lm_trim_gimbal_routines;
extern const agc_routine_table faithful_lm_trim_gimbal_routines;

static enum exec_status both(unsigned key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    apollo_overlay_state s;
    lm_flight_io io;
    agc_hardware hardware;

    if (state) {
        s = *state;
        if (device) io = *(lm_flight_io *)device;
        if (program && program->hardware) hardware = *program->hardware;
        (void)faithful_lm_trim_gimbal_routines.entries[key].call(program, &s, device);
        if (device) *(lm_flight_io *)device = io;
        if (program && program->hardware) *program->hardware = hardware;
    }
    return readable_lm_trim_gimbal_routines.entries[key].call(program, state, device);
}

#define BOTH(k)                                                                                    \
    static enum exec_status both##k(const agc_native_program *program,                             \
                                    apollo_overlay_state *state, void *device)                     \
    {                                                                                              \
        return both(k, program, state, device);                                                    \
    }
BOTH(0)
BOTH(1)
BOTH(2)
BOTH(3)

#define P16(z) (uint16_t)(016u * 1024u + ((z) - 02000u))
#define P21(z) (uint16_t)(021u * 1024u + ((z) - 02000u))

static const agc_routine_entry entries[4] = {{P16(LM_TG_ACDTC12), both0},
                                             {P21(LM_TG_GTS), both1},
                                             {P21(LM_TG_TIMEGMBL), both2},
                                             {P21(LM_TG_TIMERET), both3}};
const agc_routine_table lm_trim_gimbal_routines = {entries, 4};
