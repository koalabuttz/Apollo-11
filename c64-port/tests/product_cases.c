/* SPDX-License-Identifier: MIT */
#include "product_cases.h"
uint16_t pr_target_number(uint16_t index) {
    if(index==62u) return 282;
    if(index==63u) return 395;
    return index<32u ? (uint16_t)(index*8u) : index<48u ? (uint16_t)(256u+index-32u)
                    : (uint16_t)(3072u+index-48u);
}
void pr_prepare(product_case *c,uint16_t n,uint8_t vector) {
    unsigned i;
    static const uint8_t offset[3]={0,3,5};
    ve_prepare(c,(uint16_t)(4496u+n),0); c->preparation.mpac[7]=vector ? 077776 : 0;
    if(n<256u) {
        static const uint16_t corner[4]={0,077777,037777,040000};
        c->operand[0]=corner[(n>>4)&3u]; c->operand[1]=corner[(n>>6)&3u];
        for(i=0;i<3;++i) {
            c->preparation.mpac[offset[i]]=corner[(n+i)&3u];
            c->preparation.mpac[offset[i]+1u]=corner[((n>>2)+i)&3u];
        }
    } else if(n<400u) {
        static const uint16_t edge[12]={0,077777,1,077776,2,077775,017777,060000,020000,057777,037777,040000};
        for(i=0;i<3;++i) {
            c->preparation.mpac[offset[i]]=edge[((n-256u)/12u+i)%12u];
            c->preparation.mpac[offset[i]+1u]=edge[((n-256u)%12u+i)%12u];
        }
        c->operand[0]=edge[((n-256u)/12u+5u)%12u];
        c->operand[1]=edge[((n-256u)%12u+7u)%12u];
    } else if(n<1424u) {
        static const uint16_t low[8]={3248,5569,7265,2009,037777,040000,1,077776};
        c->operand[0]=00321; c->operand[1]=026706;
        for(i=0;i<3;++i) {
            c->preparation.mpac[offset[i]]=(uint16_t)(70u+((n+i)%64u));
            c->preparation.mpac[offset[i]+1u]=low[(n+i)%8u];
            if(n&1u) {
                c->preparation.mpac[offset[i]]^=AGC_MASK;
                c->preparation.mpac[offset[i]+1u]^=AGC_MASK;
            }
        }
    }
}
#ifndef PRODUCT_INPUT_ONLY
uint8_t pr_apply(product_case *c,uint8_t vector) {
    agc_dp factor; factor.high=c->operand[0]; factor.low=c->operand[1];
    return vector ? p66_vector_multiply(&c->preparation,&c->scale,&factor)
                  : p66_scalar_product(&c->preparation,&c->scale,&factor);
}
#endif
