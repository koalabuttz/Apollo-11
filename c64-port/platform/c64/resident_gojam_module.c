/* SPDX-License-Identifier: MIT */
/* The hardware restart (GOJAM) module, RK_MODULE_GOJAM (ABI 2 task slot;
 * the job slot in a store with a class-4 module): what the AGC's GOJAM
 * sequence does to the machine, the change tools/flight_gojam.h
 * gojam_native makes on the host (docs/FLIGHT-RESTART.md, Entry contract):
 *   Q = Z, Z = 04000;
 *   interrupts enabled, none in progress;
 *   the HANDRUPT traps cleared (rk_io where it holds them, and rk_flight);
 *   channels 5, 6, 010-014 (rk_hardware, and the CM rk_io's copies of
 *   011-014), 034 and 035 (rk_flight) zeroed;
 *   channel 033 bit 11 set.
 * The superbank and every erasable word but Q and Z stay. The pending
 * interrupt requests (the resident's rk_pending) are not reachable from a
 * module: the resident's trigger clears them. Then the main returns DONE,
 * and the next step runs the rope's own GOJAM code from 04000.
 * Value: the step's rk_module_env (rk_run_module); status stays EXEC_OK. */
#include "resident_flight.h"
uint16_t apollo_ov2_main(apollo_ov2_frame *f, uint16_t selector)
{
    rk_module_env *e = (rk_module_env *)(uintptr_t)f->value;
    rk_io *io = (rk_io *)e->io;
    agc_hardware *hw = e->program->hardware;
    flight_io *fl = (flight_io *)RK_FLIGHT_IO;
    agc_word *w = f->state->words;
    (void)selector;
    w[2] = w[5];
    w[5] = 04000u;
    io->interrupt_enabled = 1u;
    fl->interrupt_enabled = 1u;
    hw->in_interrupt = 0u;
    fl->trap31a = fl->trap31b = fl->trap32 = 0u;
#if RK_CM
    io->trap31a = io->trap31b = io->trap32 = 0u;
    io->channel11 = io->channel12 = io->channel13 = io->channel14 = 0u;
#endif
    hw->channel5 = hw->channel6 = hw->channel10 = hw->channel11 = 0u;
    hw->channel12 = hw->channel13 = hw->channel14 = 0u;
    fl->channel34 = fl->channel35 = 0u;
    hw->channel33 |= 02000u;
    return APOLLO_OV2_DONE;
}
