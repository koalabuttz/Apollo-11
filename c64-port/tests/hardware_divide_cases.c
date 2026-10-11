/* SPDX-License-Identifier: MIT */
#include "hardware_divide_cases.h"
void hw_div_input(uint32_t n,agc_register_pair *r,agc_register *divisor){
    static const uint16_t edge[16]={0,0177777,1,0177776,037777,0140000,040000,0137777,077777,0100000,077776,0100001,020000,0157777,017777,0160000};
    uint32_t seed=UINT32_C(0xd0000031)+n;
    if(n<4096u){r->a=edge[(n>>8)&15u];r->l=edge[(n>>4)&15u];*divisor=edge[n&15u];}
    else if(n<69632u){r->a=(uint16_t)((n-4096u)>>1);r->l=(n&1u)?0177777:0;*divisor=agc_extend((agc_word)(r->a&AGC_MASK));}
    else if(n<135168u){uint32_t k=n-69632u;r->a=(uint16_t)(k&0177777u);r->l=(k&1u)?040000:0137777;*divisor=agc_extend(200);}
    else{seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);r->a=(uint16_t)(seed>>8);seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);r->l=(uint16_t)(seed>>8);seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);*divisor=(uint16_t)(seed>>8);}
}
