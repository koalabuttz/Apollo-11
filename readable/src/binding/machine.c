/* SPDX-License-Identifier: MIT */
/* Pointing the machine (src/agc/agc.h) at the flight program's state, for
 * every binding (binding.h). */
#include <stddef.h>
#include "binding.h"

bool bind_machine(const agc_native_program *program, apollo_overlay_state *state, void *device)
{
    mission_io *io = device;
    agc_hardware *hardware;

    if (!program || !state || !io) return false;
    hardware = program->hardware;
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
#if LUMINARY099
    agc.dsalmout = hardware ? &hardware->channel11 : NULL;
#else
    agc.dsalmout = &io->channel11;
#endif
    agc.in_interrupt = hardware ? &hardware->in_interrupt : NULL;
    agc.fixed_memory = NULL;
    agc.channel_in = NULL;
    agc.channel_out = NULL;
    agc.leave = NULL;
    return true;
}
