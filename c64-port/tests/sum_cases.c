/* SPDX-License-Identifier: MIT */
#include "sum_cases.h"
void su_prepare(vector_case *c,uint16_t n,uint8_t operation) {
    ve_prepare(c,n,0);c->preparation.mpac[7]=operation<3u?0:077776;
    if(n>=256u&&n<272u) {
        static const uint16_t edge[16][4]={
            {037777,0,077776,0},{040000,077777,1,077777},
            {037777,077776,077776,0},{040000,1,1,077777},
            {037777,077776,077775,0},{040000,1,2,077777},
            {0,037777,0,077776},{077777,040000,077777,1},
            {037777,037777,040000,040000},{040000,040000,037777,037777},
            {0,077777,077777,0},{077777,0,0,077777},
            {1,040000,077776,037777},{077776,037777,1,040000},
            {037777,040000,040000,037777},{040000,037777,037777,040000}
        };
        static const uint8_t offsets[3]={0,3,5};unsigned component;
        for(component=0;component<(operation<3u?1u:3u);++component) {
            unsigned index=((unsigned)n-256u+component)%16u,offset=offsets[component];
            if(operation<2u) {
                c->preparation.mpac[offset]=edge[index][0];c->preparation.mpac[offset+1u]=edge[index][1];
                c->operand[2u*component]=(uint16_t)(edge[index][2]^(operation?0u:AGC_MASK));
                c->operand[2u*component+1u]=(uint16_t)(edge[index][3]^(operation?0u:AGC_MASK));
            } else {
                c->preparation.mpac[offset]=edge[index][2];c->preparation.mpac[offset+1u]=edge[index][3];
                c->operand[2u*component]=edge[index][0];c->operand[2u*component+1u]=edge[index][1];
            }
        }
    }
}
#ifndef SUM_INPUT_ONLY
uint16_t su_apply(vector_case *c,uint8_t operation) {
    agc_dp operand={c->operand[0],c->operand[1]};
    switch(operation) {
    case 0:return p66_scalar_add(&c->preparation,&c->scale,&c->elapsed,&operand);
    case 1:return p66_scalar_subtract(&c->preparation,&c->scale,&c->elapsed,&operand);
    case 2:return p66_scalar_reverse_subtract(&c->preparation,&c->scale,&c->elapsed,&operand);
    default:return p66_vector_reverse_subtract(&c->preparation,&c->scale,&c->elapsed,c->operand);
    }
}
#endif
