/* SPDX-License-Identifier: MIT */
#include "elapsed_cases.h"
static uint16_t push_number(uint16_t n) {
    return n<160u ? (uint16_t)(n-32u+60u) : n<163u ? (uint16_t)(n-160u+188u)
           : (uint16_t)(n-2467u+1299u);
}
uint8_t el_integrated(uint16_t n) {
    return (uint8_t)((n>=32u && n<163u) || n>=2467u);
}
static uint16_t random_word(uint32_t *seed) {
    *seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);
    return (uint16_t)((*seed>>16)&AGC_MASK);
}
void el_prepare(elapsed_case *c,uint16_t n) {
    uint32_t seed=UINT32_C(0x70057035)^((uint32_t)n*UINT32_C(2654435761));
    p66_preparation *s=&c->previous.previous.preparation;
    if(el_integrated(n)) ps_prepare(&c->previous,push_number(n));
    else { ps_prepare(&c->previous,(uint16_t)(275u+n%1024u)); s->mpac[7]=0; }
    c->elapsed.piptime.high=random_word(&seed); c->elapsed.piptime.low=random_word(&seed);
    c->elapsed.buf=random_word(&seed);
    if(n<16u) {
        s->mpac[0]=(n&1u) ? AGC_MASK : 0; s->mpac[1]=(n&2u) ? AGC_MASK : 0;
        c->elapsed.piptime.high=(n&4u) ? AGC_MASK : 0;
        c->elapsed.piptime.low=(n&8u) ? AGC_MASK : 0;
    } else if(n<32u) {
        /* +/- initial high overflow: both TS skips, neither TS skip,
         * only the final TS skip; then low carries and mixed-sign edges. */
        static const uint16_t edges[16][4]={
            {037777,0,077776,0},{040000,077777,1,077777},
            {037777,077776,077776,0},{040000,1,1,077777},
            {037777,077776,077775,0},{040000,1,2,077777},
            {0,037777,0,077776},{077777,040000,077777,1},
            {037777,037777,040000,040000},{040000,040000,037777,037777},
            {0,077777,077777,0},{077777,0,0,077777},
            {1,040000,077776,037777},{077776,037777,1,040000},
            {037777,040000,040000,037777},{040000,037777,037777,040000}
        };
        s->mpac[0]=edges[n-16u][0]; s->mpac[1]=edges[n-16u][1];
        c->elapsed.piptime.high=edges[n-16u][2]; c->elapsed.piptime.low=edges[n-16u][3];
    } else if(n>=163u && n<419u) {
        static const uint16_t corner[4]={0,077777,037777,040000};
        uint16_t index=(uint16_t)(n-163u);
        s->mpac[0]=corner[index&3u]; s->mpac[1]=corner[(index>>2)&3u];
        c->elapsed.piptime.high=corner[(index>>4)&3u];
        c->elapsed.piptime.low=corner[(index>>6)&3u];
    } else if(n>=419u && n<2467u) {
        s->mpac[0]=random_word(&seed); s->mpac[1]=random_word(&seed);
    }
}
uint8_t el_steps(uint16_t n) { return n>=160u && n<163u ? 4 : 1; }
void el_next(elapsed_case *c,uint16_t n,uint8_t step) {
    if(el_integrated(n)) ps_next(&c->previous,push_number(n),step);
    if(n>=160u && n<163u) {
        c->elapsed.piptime.high=0;
        c->elapsed.piptime.low=(uint16_t)(037774u+step);
    }
}
void el_words(const elapsed_case *c,uint16_t w[EL_WORD_COUNT]) {
    ps_words(&c->previous,w); w[86]=c->elapsed.piptime.high;
    w[87]=c->elapsed.piptime.low; w[88]=c->elapsed.buf;
}
void el_from_words(elapsed_case *c,const uint16_t w[EL_WORD_COUNT]) {
    ps_from_words(&c->previous,w); c->elapsed.piptime.high=w[86];
    c->elapsed.piptime.low=w[87]; c->elapsed.buf=w[88];
}
#ifndef ELAPSED_INPUT_ONLY
void el_prefix(elapsed_case *c,uint16_t n) {
    ps_prefix(&c->previous,push_number(n)); (void)ps_apply(&c->previous);
}
uint8_t el_apply(elapsed_case *c) {
    return p66_elapsed_time(&c->previous.previous.preparation,
                            &c->previous.previous.scratch,&c->elapsed);
}
#endif
