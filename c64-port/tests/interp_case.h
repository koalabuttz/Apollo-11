/* SPDX-License-Identifier: MIT */
#ifndef INTERP_CASE_H
#define INTERP_CASE_H
/* Test cases for compiled-routine milestones (tools/interp_compile.py):
 * an erasable image from a seed plus edits, the io latches, and the run
 * through the control-transfer trampoline to the first point no routine of
 * the program covers. The yaAGC oracle (tools/interp_routine_reference.c),
 * the host differential, the host test and the C64 diagnostic share it. */
#include "agc_transfer.h"
#include "p66_bias.h"
typedef struct { uint16_t addr; agc_word word; } irc_edit;
typedef struct {
    uint32_t seed; uint8_t cm, interrupt_enabled; agc_word superbank;
    uint16_t nedits; const irc_edit *edits;
} irc_case;
/* Fill all 2048 words from the seed (A and L sign-extended, the special
 * and counter registers 010-061 and ZERO cleared), then apply the edits. */
void irc_build(apollo_overlay_state *, const irc_case *);
enum { IRC_STOPPED = 0, IRC_REFUSED = 1, IRC_FAILED = 2 };
/* Run from the image's point until a point no routine covers (the case's
 * stop), at most max_steps routines. Returns IRC_STOPPED at an unmapped
 * point, IRC_REFUSED when a routine refuses, IRC_FAILED otherwise; ie gets
 * the interrupt-enable latch. */
unsigned irc_run(const agc_native_program *, apollo_overlay_state *, const irc_case *,
                 uint16_t max_steps, uint16_t *ie);
/* Running sums over the kind, the 2048 words (A, L 16-bit) and ie. */
uint32_t irc_digest(unsigned kind, const apollo_overlay_state *, uint16_t ie);
#endif
