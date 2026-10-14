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
#include "agc_input_channels.h"
typedef struct { uint16_t addr; agc_word word; } irc_edit;
typedef struct {
    uint32_t seed; uint8_t cm, interrupt_enabled; agc_word superbank;
    uint16_t nedits; const irc_edit *edits;
} irc_case;
/* Channel edits (Lane G): an edit whose addr is IRC_CHANNEL|k sets I/O
 * channel k (one of irc_ch_num) instead of an image word. A case with any
 * channel edit runs with those channel values (the rest 0) in the CM io
 * record and in yaAGC's channels, and its digest also covers the channels
 * and the channel-13 traps after the run; a case without one is unchanged. */
#define IRC_CHANNEL 0x8000u
/* Counter edit (Lane G): a case holding an edit whose addr is IRC_COUNTERS
 * (its word unused) also fills 010-061 (the interrupt save words, the
 * editing registers and the counters: TIME1-TIME6, the CDUs, PIPAs and the
 * rest) from the seed, before its other edits; without one they stay 0. */
#define IRC_COUNTERS 0x4000u
/* Both edits work on the host; a C64 diagnostic carries their code (about
 * 1.3 KB, which the bigger groups of other files need for their soft stack)
 * only when built with -DIRC_CHANNELS, as the Lane G specs' are. */
#if !defined(__mos__) || defined(IRC_CHANNELS)
#define IRC_IO 1
#else
#define IRC_IO 0
#endif
enum { IRC_NCH = 10 };
extern const uint8_t irc_ch_num[IRC_NCH];
extern agc_word irc_ch[IRC_NCH], irc_trap[3];
extern uint8_t irc_ch_on;
/* Fill all 2048 words from the seed (A and L sign-extended, the special
 * and counter registers 010-061 and ZERO cleared unless the case has the
 * IRC_COUNTERS edit), then apply the edits. */
void irc_build(apollo_overlay_state *, const irc_case *);
/* The input channels the case latches, from its seed (a stream apart from
 * the image's): LOSCALAR (channel 4) 37777 in a quarter of the cases, else random.
 * Only routines of 'channel_inputs' specs read them; the oracle sets them
 * only under IRC_INPUTS, and a C64 diagnostic carries the code only when
 * built with -DIRC_INPUTS. */
#if !defined(__mos__) || defined(IRC_INPUTS)
#define IRC_INPUTS_ON 1
void irc_inputs(const irc_case *, agc_input_channels *);
#else
#define IRC_INPUTS_ON 0
#endif
enum { IRC_STOPPED = 0, IRC_REFUSED = 1, IRC_FAILED = 2 };
/* Run from the image's point until a point no routine covers (the case's
 * stop), at most max_steps routines. Returns IRC_STOPPED at an unmapped
 * point, IRC_REFUSED when a routine refuses, IRC_FAILED otherwise; ie gets
 * the interrupt-enable latch. */
unsigned irc_run(const agc_native_program *, apollo_overlay_state *, const irc_case *,
                 uint16_t max_steps, uint16_t *ie);
/* The CM io record's channels and traps from irc_ch/irc_trap (save 0), or
 * back into them (save 1). */
void irc_cm_io(void *cm_io, agc_hardware *h, unsigned save);
const agc_native_program *irc_cm_begin(const agc_native_program *p, agc_native_program *q, void *cm_io);
/* Luminary: the program's agc_hardware outputs (h may be NULL) and the run's
 * latched inputs, from irc_ch/irc_trap (save 0) or back into them (save 1). */
void irc_lm_io(agc_hardware *h, agc_input_channels *in, unsigned save);
/* Running sums over the kind, the 2048 words (A, L 16-bit) and ie, then the
 * channels and traps when the case set channels. */
uint32_t irc_digest(unsigned kind, const apollo_overlay_state *, uint16_t ie);
#endif
