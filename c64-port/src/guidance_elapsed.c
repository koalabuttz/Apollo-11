/* SPDX-License-Identifier: MIT */
#include "guidance_elapsed.h"
static agc_register overflow_indicator(int overflow) {
    return overflow>0 ? 1 : agc_extend(agc_from_int(-1));
}
uint8_t p66_elapsed_time(p66_preparation *s,p66_scale_scratch *scale,
                         p66_elapsed_state *elapsed) {
    agc_register_pair *r=&s->rate.registers;
    agc_dp result;
    uint8_t path=0;
    result.high=s->mpac[0]; result.low=s->mpac[1];
    r->a=agc_extend(agc_negate(elapsed->piptime.high));
    r->l=agc_extend(agc_negate(elapsed->piptime.low));
    agc_double_add(r,&result);
    if(r->a!=0) {
        agc_word limit=r->a==1 ? 037777 : 040000;
        agc_register a;
        int overflow;
        path=1; elapsed->buf=limit;
        /* INDEX +/-1; CS LIMITS picks the same-sign maximum. AUG forces
         * a register overflow, then ADS/TS 7 realign the low/high signs.
         * A skipped CAF ZERO carries +/-1 into the high-word adjustment. */
        a=agc_add_register(agc_extend(limit),overflow_indicator(r->a==1 ? 1 : -1));
        a=agc_add_register(a,agc_extend(result.low)); result.low=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow!=0) { path|=2u; a=overflow_indicator(overflow); }
        else a=0;
        a=agc_add_register(a,agc_extend(limit));
        a=agc_add_register(a,agc_extend(result.high)); result.high=agc_correct(a);
        overflow=agc_overflow(a);
        if(overflow!=0) {
            path|=4u; a=overflow_indicator(overflow);
            scale->ovfind=agc_correct(a);
        }
        r->a=a; /* Retain raw A on the no-skip return, +/-1 otherwise. */
    }
    s->mpac[0]=result.high; s->mpac[1]=result.low;
    return path;
}
