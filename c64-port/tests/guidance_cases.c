/* SPDX-License-Identifier: MIT */
#include "guidance_cases.h"
static const uint16_t edges[12] = {
    0,077777,1,077776,2,077775,017777,060000,020000,057777,037777,040000
};
static uint16_t extended(uint16_t word) {
    return (uint16_t)(word | (uint16_t)((word & 040000u) << 1));
}
static uint16_t random_word(uint32_t *seed) {
    *seed = *seed * UINT32_C(1664525) + UINT32_C(1013904223);
    return (uint16_t)(*seed >> 16);
}
void gn_prepare(guidance_case *c, uint16_t number) {
    p66_rate_state *s = &c->state;
    uint16_t n = number;
    s->count = 012345; s->scale = 023456;
    s->desired.high = 034567; s->desired.low = 045670;
    s->registers.a = 012345; s->registers.l = extended(065432);
    if (n < 144u) {
        c->operation = GN_MP;
        s->registers.a = extended(edges[n / 12u]); s->scale = edges[n % 12u];
    } else if (n < 400u) {
        static const uint16_t corners[4] = {0,077777,037777,040000};
        n = (uint16_t)(n - 144u); c->operation = GN_DAS;
        s->registers.a = extended(corners[n & 3u]);
        s->registers.l = extended(corners[(n >> 2) & 3u]);
        s->desired.high = corners[(n >> 4) & 3u];
        s->desired.low = corners[(n >> 6) & 3u];
    } else if (n < GN_SCALAR_COUNT) {
        n = (uint16_t)(n - 400u); c->operation = GN_ROD;
        s->count = edges[n / 12u]; s->scale = edges[n % 12u];
        s->desired.high = edges[(n + 3u) % 12u];
        s->desired.low = edges[(n / 12u + 5u) % 12u];
    } else if (n < GN_TARGET_COUNT) {
        c->operation = GN_ROD;
        s->count = 0;
        s->scale = n == 544u ? 1 : n == 545u ? 037777 : 040000;
        s->desired.high = 077776; s->desired.low = n == 544u ? 077777 : 037777;
    } else {
        uint32_t seed = UINT32_C(0x1b5d6474) ^ ((uint32_t)n * UINT32_C(2654435761));
        c->operation = (enum gn_operation)((n - GN_TARGET_COUNT) / 4096u);
        s->count = (agc_word)(random_word(&seed) & AGC_MASK);
        s->scale = (agc_word)(random_word(&seed) & AGC_MASK);
        s->desired.high = (agc_word)(random_word(&seed) & AGC_MASK);
        s->desired.low = (agc_word)(random_word(&seed) & AGC_MASK);
        s->registers.a = random_word(&seed); /* all 16-bit overflow patterns */
        s->registers.l = extended((agc_word)(random_word(&seed) & AGC_MASK));
    }
}
uint8_t gn_steps(uint16_t number) {
    return number >= GN_SCALAR_COUNT && number < GN_TARGET_COUNT ? 8 : 1;
}
void gn_next(guidance_case *c, uint16_t number, uint8_t step) {
    static const uint16_t clicks[8] = {1,1,077776,0,077777,3,077774,0};
    if (number >= GN_SCALAR_COUNT && number < GN_TARGET_COUNT) c->state.count = clicks[step];
}
void gn_words(const guidance_case *c, uint16_t words[6]) {
    words[0] = c->state.count; words[1] = c->state.scale;
    words[2] = c->state.desired.high; words[3] = c->state.desired.low;
    words[4] = c->state.registers.a; words[5] = c->state.registers.l;
}
#ifndef GUIDANCE_INPUT_ONLY
void gn_apply(guidance_case *c) {
    if (c->operation == GN_MP) c->state.registers = agc_multiply(c->state.registers.a, c->state.scale);
    else if (c->operation == GN_DAS) agc_double_add(&c->state.registers, &c->state.desired);
    else p66_rate_update(&c->state);
}
#endif
