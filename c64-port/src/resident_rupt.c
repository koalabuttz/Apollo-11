/* SPDX-License-Identifier: MIT */
/* The flight resident's AGC timers and interrupt entry (resident_rupt.h). */
#include "resident_rupt.h"

/* yaAGC CounterPINC: +1 in one's complement, 037777 overflows to +0, -0
 * steps to +1. */
static uint8_t pinc(agc_word *w) {
    agc_word v=*w;
    if(v==037777u){*w=0;return 1;}
    v=(agc_word)((v+1u)&AGC_MASK);
    *w=v?v:1u;
    return 0;
}

void rk_timer_tick(apollo_overlay_state *s, agc_hardware *hw, uint16_t *pending) {
    uint8_t i;
    agc_word *w=s->words;
    if(pinc(&w[030])) *pending|=RK_RUPT_BIT(RK_RUPT_T5);
    if(pinc(&w[027])) *pending|=RK_RUPT_BIT(RK_RUPT_T4);
    if(pinc(&w[025])) pinc(&w[024]);
    if(pinc(&w[026])) *pending|=RK_RUPT_BIT(RK_RUPT_T3);
    /* yaAGC CounterDINC: toward zero by one through the one's complement
     * adder, so +1 and -1 both step to -0; at +0 or -0 the ZOUT. */
    for(i=0;i<16u && (hw->channel13&040000u);++i) {
        agc_word v=w[031];
        if(v==0u||v==AGC_MASK){*pending|=RK_RUPT_BIT(RK_RUPT_T6);hw->channel13&=037777u;}
        else w[031]=(agc_word)(v==1u?AGC_MASK:(v&AGC_SIGN)?v+1u:v-1u);
    }
}

static uint8_t word_at(const agc_native_program *p, const apollo_overlay_state *s, agc_word superbank,
                       agc_word z, agc_word *word) {
    if(z>=02000u)
        return p->fixed_data && p->fixed_data(p,agc_fixed_physical(z,s->words[4],superbank),word);
    *word=s->words[z<01400u ? z : (uint16_t)(((s->words[3]>>8)&7u)*0400u+(z&0377u))];
    return 1;
}

uint8_t rk_rupt_take(const agc_native_program *p, apollo_overlay_state *s, agc_word superbank,
                     agc_word interrupt_enabled, uint16_t *pending) {
    agc_word z=(agc_word)(s->words[5]&07777u),word;
    uint8_t i;
    if(!(*pending&03776u)||!p->hardware||p->hardware->in_interrupt||!interrupt_enabled||agc_overflow(s->words[0]))
        return 0;
    if(!word_at(p,s,superbank,z,&word)) return 0;
    word&=AGC_MASK;
    if(word==3u||word==4u||word==6u) return 0;
    for(i=RK_RUPT_T6;!(*pending&RK_RUPT_BIT(i));++i) {}
    *pending&=(uint16_t)~RK_RUPT_BIT(i);
    s->words[015]=(agc_word)(z+1u);
    s->words[017]=word;
    s->words[5]=(agc_word)(04000u+4u*i);
    p->hardware->in_interrupt=1;
    return i;
}
