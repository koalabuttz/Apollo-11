/* SPDX-License-Identifier: MIT */
/* include/agc_channel_ops.h against yaAGC's channel instructions
 * (agc_engine.c READ..RXOR, ReadIO, WriteIO, CpuWriteIO), by hand. The
 * routine differentials of 'channels' specs compare it with yaAGC itself. */
#include <stdio.h>
#include "agc_channel_ops.h"
static unsigned fails;
static void eq(const char *what, unsigned got, unsigned want) {
    if (got != want) {
        printf("FAIL %s: %06o, want %06o\n", what, got, want);
        ++fails;
    }
}
int main(void) {
    static apollo_overlay_state s;
    agc_hardware hw = {0};
    agc_input_channels in = {0};
    agc_word sb = 0100;
    /* WOR CHAN13 with BIT12: A keeps bit 12, the 31A trap is armed and
     * reads back 0, a later WRITE does not clear it. */
    s.words[0] = 04000; hw.channel13 = 040000;
    eq("wor 13", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WOR, 013), 1);
    eq("wor 13 A", s.words[0], 0144000); eq("wor 13 hw", hw.channel13, 044000);
    eq("read 13", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_READ, 013), 1);
    eq("read 13 A", s.words[0], 0140000);
    s.words[0] = 0; (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 013);
    eq("write 13 hw", hw.channel13, 004000);
    /* WAND CHAN12: A overflow-corrected (-0 in 16 bits), result sign-extended. */
    s.words[0] = 0177777; hw.channel12 = 052345;
    (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WAND, 012);
    eq("wand 12 A", s.words[0], 0152345); eq("wand 12 hw", hw.channel12, 052345);
    /* RXOR CHAN6 leaves the channel; READ CHAN30 sign-extends. */
    s.words[0] = 1; hw.channel6 = 3; (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_RXOR, 06);
    eq("rxor 6 A", s.words[0], 2); eq("rxor 6 hw", hw.channel6, 3);
    in.channel30 = 040001; (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_READ, 030);
    eq("read 30 A", s.words[0], 0140001);
    /* WRITE CHAN33 only sets bits 11-15; WRITE leaves A and corrects L. */
    s.words[0] = 0; s.words[1] = 040000; in.channel33 = 001234;
    (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 033);
    eq("write 33", in.channel33, 077234); eq("write 33 A", s.words[0], 0); eq("write 33 L", s.words[1], 0);
    /* SUPERBNK: READ gives the superbank; a write keeping it runs, one
     * changing it refuses, as do writes to 3, 4, 016, channel 010 and a
     * missing agc_hardware. */
    (void)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_READ, 07); eq("read 7 A", s.words[0], 0100);
    s.words[0] = 0160; eq("write 7 more bits", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 07), 0);
    s.words[0] = 0100; eq("write 7 same", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 07), 1);
    s.words[0] = 0; eq("write 7 change", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 07), 0);
    eq("write 4", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_WRITE, 04), 0);
    eq("read 10", (unsigned)agc_ch_op(&s, &hw, &in, &sb, AGC_CH_READ, 010), 0);
    eq("no hardware", (unsigned)agc_ch_op(&s, 0, &in, &sb, AGC_CH_WOR, 05), 0);
    eq("refusals change nothing", s.words[0], 0);
    eq("superbank", sb, 0100);
    puts(fails ? "agc_channel_ops: FAIL" : "agc_channel_ops: PASS");
    return fails != 0;
}
