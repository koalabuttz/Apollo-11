/* SPDX-License-Identifier: MIT */
#ifndef AGC_CHANNEL_OPS_H
#define AGC_CHANNEL_OPS_H
#include "agc_transfer.h"
#include "agc_image_ops.h"
#include "agc_input_channels.h"
/* The I/O channel model of compiled Luminary routines (spec key 'channels':
 * 'all', tools/interp_compile.py and tools/basic_compile.py): a channel
 * instruction as yaAGC's CPU runs it. op is the instruction's bits 10-12
 * (READ 0, WRITE 1, RAND 2, WAND 3, ROR 4, WOR 5, RXOR 6), ch its channel.
 * A is overflow-corrected first; READ, RAND, WAND, ROR, WOR and RXOR leave
 * the result in A, sign-extended; WRITE, WAND and WOR also store it. L is
 * overflow-corrected after each, as after every instruction.
 *
 *  - Output channels 5, 6, 011, 012, 013 and 014 are program->hardware's
 *    latches (agc_hardware, the Kernel lane's field names). Channel 013's
 *    bits 12-14 arm the HANDRUPT traps (31A, 31B, 32): they read back 0,
 *    and hw->channel13 holds them only until the next timer step, where
 *    rk_hand_due (resident_rupt.h) moves them to the program's trap arms
 *    (flight_io trap31a, trap31b, trap32) and clears them.
 *  - Inputs 3, 4, 016 and 030-033 are the run's latched agc_input_channels
 *    (AGC_CHANNEL). A write to 030-032 stores, as yaAGC's does; a write to
 *    033 only sets its bits 11-15 again (the CPU's latch reset); a write to
 *    3, 4 or 016 is refused.
 *  - Channel 7 (SUPERBNK) is the run's superbank. A write that would change
 *    it is refused: the trampoline keeps the caller's.
 *
 * Returns 0, changing nothing, to refuse (the routine then returns
 * EXEC_OUT_OF_SCOPE): also for a channel not modelled here (010, 015, ...)
 * or when the record it needs is NULL. The tests compare the outputs and
 * 030-033 with yaAGC's after each run (tests/interp_case.h). */
enum agc_ch_opcode { AGC_CH_READ, AGC_CH_WRITE, AGC_CH_RAND, AGC_CH_WAND, AGC_CH_ROR, AGC_CH_WOR, AGC_CH_RXOR };
static __attribute__((unused)) int agc_ch_op(apollo_overlay_state *s, agc_hardware *hw, agc_input_channels *in,
                                             agc_word *superbank, unsigned op, unsigned ch) {
    agc_word *c = 0, a, v, now;
    switch (ch) {
    case 05: if (hw) c = &hw->channel5; break;
    case 06: if (hw) c = &hw->channel6; break;
    case 011: if (hw) c = &hw->channel11; break;
    case 012: if (hw) c = &hw->channel12; break;
    case 013: if (hw) c = &hw->channel13; break;
    case 014: if (hw) c = &hw->channel14; break;
    case 03: if (in) c = &in->channel3; break;
    case 04: if (in) c = &in->channel4; break;
    case 016: if (in) c = &in->channel16; break;
    case 030: if (in) c = &in->channel30; break;
    case 031: if (in) c = &in->channel31; break;
    case 032: if (in) c = &in->channel32; break;
    case 033: if (in) c = &in->channel33; break;
    case 07: c = superbank; break;
    default: break;
    }
    if (!c || op > AGC_CH_RXOR)
        return 0;
    a = (agc_word)(((s->words[0] >> 1) & 040000u) | (s->words[0] & 037777u));
    now = ch == 07u ? (agc_word)(*c & 0160u) : ch == 013u ? (agc_word)(*c & 043777u) : *c;
    switch (op) {
    case AGC_CH_READ: v = now; break;
    case AGC_CH_WRITE: v = a; break;
    case AGC_CH_RAND: case AGC_CH_WAND: v = (agc_word)(a & now); break;
    case AGC_CH_ROR: case AGC_CH_WOR: v = (agc_word)(a | now); break;
    default: v = (agc_word)(a ^ now); break;
    }
    v &= 077777u;
    if (op == AGC_CH_WRITE || op == AGC_CH_WAND || op == AGC_CH_WOR) {
        if (ch == 03u || ch == 04u || ch == 016u || (ch == 07u && (v & 0160u) != now))
            return 0;
        if (ch == 013u)
            *c = (agc_word)((*c & 034000u) | v);
        else if (ch == 033u)
            *c = (agc_word)(*c | 076000u);
        else if (ch != 07u)
            *c = v;
    }
    if (op != AGC_CH_WRITE) {
        ai_caf(s, v);
    } else {
        /* (L overflow-corrected and sign-extended, as after every instruction) */
        a = (agc_word)(((s->words[1] >> 1) & 040000u) | (s->words[1] & 037777u));
        s->words[1] = (agc_register)(a | ((a << 1) & 0100000u));
    }
    return 1;
}
#endif
