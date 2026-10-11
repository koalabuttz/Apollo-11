/* SPDX-License-Identifier: MIT */
/* Supplement the frozen corner corpus with every ordinary MPAC high word.
 * Reuse its varied minor/third words, banks, MODE and external IRQ contexts.
 * The original corner factory and proof identities remain unchanged. */
#define inverse_trig_input inverse_trig_corner_input
#include "inverse_trig_cases.c"
#undef inverse_trig_input
void inverse_trig_input(apollo_overlay_state *m, agc_word *irq,
                        enum inverse_trig_mission mission, uint32_t n)
{
    inverse_trig_corner_input(m, irq, mission, n);
    m->words[0154] = (agc_word)(n >> 1);
}
