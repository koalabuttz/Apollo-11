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

/* The word of fixed memory at a physical address, for a routine that reads
 * the word after its caller's TC (a CADR, an alarm's code). A program's
 * operand sets name those addresses: the call sites each table of
 * routines serves. A case test's set gives each word too, so that the test
 * can make one up or leave one out. The flight program's set gives the
 * address alone (WORD_IN_ROPE in the word's place), and the word is read
 * from the rope image, the program's fixed_data. False if the set does not
 * name the address, or the program has no rope to read. */
#define WORD_IN_ROPE UINT16_C(0100000) /* no 15-bit word */
bool fixed_word_at(const agc_native_program *program, unsigned physical, agc_word *value);
/* The word at address q that the TC at q-1 passes inline (the 2CADR after
 * a TC NOVAC): an erasable word of the state above the registers, or a
 * word of fixed memory under the state's FBANK and the superbank. */
bool inline_word(const agc_native_program *program, const apollo_overlay_state *state,
                 unsigned superbank, word q, agc_word *out);

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

/* After an RTB routine an order pair runs (src/rtb_op_codes/), which
 * ends where the listing leaves the file: on through the routines it
 * leaves for, to the pair's DANZIG (rtb_run_on.c). */
void rtb_run_on(void);

/* Keys: physical addresses of fixed-fixed (04000-07777), bank 01, and a
 * word at 02000-03777 in any switched bank (040-043 are 30-33 with the
 * superbank bit). */
#define FF(z)             (uint16_t)(2048u + ((z) - 04000u))
#define B1(z)             (uint16_t)(1024u + ((z) - 02000u))
#define SWITCHED(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define COUNT(table)      (uint16_t)(sizeof(table) / sizeof(table)[0])

#endif
