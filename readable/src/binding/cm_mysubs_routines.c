/* SPDX-License-Identifier: MIT */
/* The readable SMALLMP and DPADD (src/cm_dap/mysubs.c) as routines of the
 * CM's flight program, under the table name and keys of the faithful
 * port's (c64-port/src/cm_mysubs_routines.c): cm_mysubs_routines, SMALLMP
 * at 21,2042 and DPADD at 21,2054.
 *
 * Plumbing only: point the machine at the program's state and take the
 * states the faithful routines take: FB bank 21, EBANK E6 (KMPAC's), BBANK
 * agreeing, ZERO +0, and a return address in fixed memory for TC Q. */
#include "binding.h"
#include "cm_mysubs_routines.h"
#include "../cm_dap/cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

#define ZERO_REGISTER 7u

/* The file's bank. */
#define MYSUBS_BANK 021u

static bool takes(void)
{
    return FBANK == (word)(MYSUBS_BANK << 10) && EBANK == 03000u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && agc.memory[ZERO_REGISTER] == 0u &&
           Q >= 02000u && Q <= 07777u;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static mission_io no_channels; /* the routines touch none: the caller may pass none */

    if (!device) device = &no_channels;
    if (!bind_machine(program, state, device) || !takes()) return EXEC_OUT_OF_SCOPE;
    if (Z == SMALLMP)
        smallmp();
    else if (Z == DPADD)
        dpadd();
    else
        return EXEC_OUT_OF_SCOPE;
    return EXEC_OK;
}

static const agc_routine_entry routines[] = {
    {(uint16_t)(MYSUBS_BANK * 1024u + (SMALLMP & 01777u)), run},
    {(uint16_t)(MYSUBS_BANK * 1024u + (DPADD & 01777u)), run},
};
const agc_routine_table cm_mysubs_routines = {routines, COUNT(routines)};

#endif
