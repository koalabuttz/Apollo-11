/* SPDX-License-Identifier: MIT */
/* What the routine bindings share: pointing the machine (src/agc/agc.h) at
 * the flight program's state, and the keys of its routine tables. Plumbing
 * only (README.md, src/binding/). */
#ifndef READABLE_BINDING_H
#define READABLE_BINDING_H

#include "resident_kernel.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#endif
#include "../agc/agc.h"

/* Point agc at the program's memory and channels. False if any is missing. */
bool bind_machine(const agc_native_program *program, apollo_overlay_state *state, void *device);

/* One pass of the readable interpreter (interpreter_core.c) over a table
 * of interpretive code in the listing's notation (src/interpreter/
 * notation.h), reading the table's words in place of the rope's. */
struct interpretive_code;
enum exec_status interpret_listing(const agc_native_program *program, apollo_overlay_state *state,
                                   void *device, const struct interpretive_code *code);
/* The same pass from the states the faithful port's lanec modules take
 * (c64-port/src/lanec_ev_lmi.c): any work area the caller has checked
 * (FIXLOC anywhere in unswitched erasable, not only a VAC area), the
 * superbank by its bit 0100 alone, and Q left as the AGC leaves it (a
 * switch order's TS Q copies all 16 bits of A). */
enum exec_status interpret_listing_lanec(const agc_native_program *program,
                                         apollo_overlay_state *state, void *device,
                                         const struct interpretive_code *code);
/* The same pass over code that ends before DANZIG reads NEWJOB (a CALL or
 * an EXIT that ends the pass at its first order, as E/CALL's do in
 * SYSTEM_TEST_STANDARD_LEAD_INS): the faithful port's routines for such
 * words (c64-port/src/standard_lead_interp.c) take any NEWJOB, and so does
 * this pass. */
enum exec_status interpret_listing_any_newjob(const agc_native_program *program,
                                              apollo_overlay_state *state, void *device,
                                              const struct interpretive_code *code);

/* Keys: physical addresses of fixed-fixed (04000-07777) and bank 01. */
#define FF(z)        (uint16_t)(2048u + ((z) - 04000u))
#define B1(z)        (uint16_t)(1024u + ((z) - 02000u))
#define COUNT(table) (uint16_t)(sizeof(table) / sizeof(table)[0])

#endif
