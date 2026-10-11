/* SPDX-License-Identifier: MIT */
#include "guidance_prepare.h"
void p66_snapshot(p66_preparation *s) {
    unsigned i;
    /* DCA/DXCH and CA/XCH leave the previous OLDPIPA values in RUPTREG.
     * Their intermediate register debris is overwritten by DCA TIME2. */
    for (i = 0; i < 3; ++i) {
        s->ruptreg[i] = s->old_pipa[i];
        s->old_pipa[i] = s->pipa[i];
    }
    s->rate.registers.a = agc_extend(s->this_piptime.high);
    s->rate.registers.l = agc_extend(s->this_piptime.low);
    s->this_piptime = s->time;
}
/* TS to ordinary memory: store corrected A; overflow returns +/-1 in A
 * and skips one instruction. Nonoverflow leaves the full A unchanged. */
static uint8_t store_skip(agc_register *a, agc_word *word) {
    int overflow = agc_overflow(*a);
    *word = agc_correct(*a);
    if (overflow == 0) return 0;
    *a = overflow > 0 ? 1 : agc_extend(agc_from_int(-1));
    return 1;
}
uint8_t p66_vector_inputs(p66_preparation *s) {
    static const uint8_t component[3] = {0,3,5};
    agc_register *a = &s->rate.registers.a;
    unsigned i;
    uint8_t skip = 0, mask = 0;
    for (i = 0; i < 3; ++i) {
        if (!skip) *a = agc_extend(s->old_pipa[i]);
        *a = agc_add_register(*a,agc_extend(s->pipatmp[i]));
        skip = store_skip(a,&s->mpac[component[i]]);
        mask = (uint8_t)(mask | (uint8_t)(skip << i));
    }
    for (i = 0; i < 3; ++i) {
        if (!skip) *a = agc_extend(agc_negate(s->old_pipa[i]));
        *a = agc_add_register(*a,agc_extend(s->tem[i]));
        *a = agc_add_register(*a,agc_extend(s->ruptreg[i]));
        skip = store_skip(a,&s->delvrod[i * 2]);
        mask = (uint8_t)(mask | (uint8_t)(skip << (i + 3u)));
    }
    /* A final delta overflow skips CAF ZERO. Its +/-1 is then stored into
     * every low MPAC component and TEM word, exactly like the source. */
    if (!skip) *a = 0;
    s->mpac[1] = s->mpac[4] = s->mpac[6] = agc_correct(*a);
    for (i = 0; i < 3; ++i) s->tem[i] = agc_correct(*a);
    *a = agc_extend(agc_from_int(-1));
    s->mpac[7] = agc_correct(*a); /* MODE aliases MPAC +7 */
    return mask;
}
uint8_t p66_prepare(p66_preparation *s) {
    p66_rate_update(&s->rate);
    p66_snapshot(s);
    return p66_vector_inputs(s);
}
