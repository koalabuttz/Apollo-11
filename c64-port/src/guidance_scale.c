/* SPDX-License-Identifier: MIT */
#include "guidance_scale.h"
/* Independent arithmetic translation of DMPSUB/VROUND, using the already
 * verified normal-memory MP and DAS primitives. No instruction decoding. */
static void accumulate(agc_register_pair *r, agc_word *high, agc_word *low) {
    agc_dp pair;
    pair.high=*high; pair.low=*low;
    agc_double_add(r,&pair); *high=pair.high; *low=pair.low;
}
static agc_register indicator(int overflow) {
    return overflow>0 ? 1 : agc_extend(agc_from_int(-1));
}
static void component(agc_word *high, agc_word *low, agc_word *third,
                      agc_register_pair *r, p66_scale_scratch *scratch) {
    const agc_word scalar_high=00321, scalar_low=026706;
    agc_word minor=*low, major=*high;
    agc_register a;
    int overflow;
    /* The minor/minor product's low word is discarded, exactly as DMPSUB.
     * Two cross products accumulate into the middle/third pair before the
     * major/major product is added into the major/middle pair. */
    *low=0;
    *r=agc_multiply(agc_extend(minor),scalar_low); *third=agc_correct(r->a);
    *r=agc_multiply(agc_extend(scalar_low),major);
    accumulate(r,low,third);
    scratch->mptemp=scalar_high;
    *r=agc_multiply(agc_extend(minor),scalar_high);
    accumulate(r,low,third);
    *high=agc_correct(r->a);
    *r=agc_multiply(agc_extend(major),scalar_high);
    accumulate(r,high,low);
    /* VROUND tests DOUBLE of the third word with TS L. On overflow the
     * skipped return adds +/-1, propagating another TS overflow if needed.
     * DXCH on the vector exit corrects/extends both surviving registers. */
    a=agc_extend(*third); *third=agc_correct(r->a);
    a=agc_add_register(a,a); r->l=agc_extend(agc_correct(a));
    overflow=agc_overflow(a);
    if(overflow!=0) {
        a=agc_add_register(indicator(overflow),agc_extend(*low));
        *low=agc_correct(a); overflow=agc_overflow(a);
        if(overflow!=0) {
            a=agc_add_register(indicator(overflow),agc_extend(*high));
            *high=agc_correct(a); overflow=agc_overflow(a);
            if(overflow!=0) {
                a=indicator(overflow); scratch->ovfind=agc_correct(a);
            }
        }
    }
    r->a=agc_extend(agc_correct(a));
}
void p66_scale_velocity(p66_preparation *s, p66_scale_scratch *scratch) {
    static const uint8_t positions[3]={0,3,5};
    uint8_t i;
    for(i=0;i<3;++i) {
        uint8_t p=positions[i];
        component(&s->mpac[p],&s->mpac[p+1u],&s->mpac[2],
                  &s->rate.registers,scratch);
    }
}
