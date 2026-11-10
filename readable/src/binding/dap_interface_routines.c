/* SPDX-License-Identifier: MIT */
/* The readable DAP_INTERFACE_SUBROUTINES (src/dap_interface/), the LM's,
 * as routines of its flight program, under the table name and keys of the
 * faithful port's (c64-port/src/lm_dap_interface.c):
 * lm_dap_interface_routines, 12 keys, every label a caller reaches and
 * every word a call returns to, all in bank 20.
 *
 * Plumbing only. A key's function runs until control leaves it (a TC, or
 * a return to the caller). The faithful routines take any state with FB
 * bank 20; they refuse, as the readable ones do, a return (TC Q, TC
 * RUPTREG1, TC RUPTREG2) to a word outside fixed memory and ALLCOAST +3
 * with A overflowed, leaving the state and the io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "lm_dap_interface.h"
#include "../dap_interface/dap_interface.h"
#include "../agc/names.h"

#if LUMINARY099

#define DAP_BANK 020u /* bank 20 */
#define B20(z)   (uint16_t)(DAP_BANK * 1024u + ((z) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, void (*routine)(void))
{
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if ((FBANK & 076000u) != (word)(DAP_BANK << 10)) return EXEC_OUT_OF_SCOPE;
    agc.leave = leave;
    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name)                                                                                  \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, name);                                                  \
    }
KEY(restordb)
KEY(restordb_plus1)
KEY(setmaxdb)
KEY(callaccs_after_novac)
KEY(setmindb)
KEY(pflitedb)
KEY(pflitedb_after_zatteror)
KEY(zatteror)
KEY(stoprate)
KEY(allcoast)
KEY(allcoast_after_stoprate)
KEY(allcoast_after_restordb)

/* By address, as the faithful table has them. */
static const agc_routine_entry keys[] = {
    {B20(RESTORDB), key_restordb},
    {B20(RESTORDB + 1u), key_restordb_plus1},
    {B20(SETMAXDB), key_setmaxdb},
    {B20(CALLACCS + 4u), key_callaccs_after_novac},
    {B20(SETMINDB), key_setmindb},
    {B20(PFLITEDB), key_pflitedb},
    {B20(PFLITEDB + 3u), key_pflitedb_after_zatteror},
    {B20(ZATTEROR), key_zatteror},
    {B20(STOPRATE), key_stoprate},
    {B20(ALLCOAST), key_allcoast},
    {B20(ALLCOAST + 3u), key_allcoast_after_stoprate},
    {B20(ALLCOAST + 021u), key_allcoast_after_restordb},
};

const agc_routine_table lm_dap_interface_routines = {keys, sizeof keys / sizeof keys[0]};

#endif
