/* SPDX-License-Identifier: MIT */
#ifndef DP_DIVIDE_CASES_H
#define DP_DIVIDE_CASES_H
#include <stdint.h>
#define DP_DIVIDE_CASES 163840u
static uint16_t dp_word(uint32_t *r) { *r=*r*1664525u+1013904223u; return (uint16_t)((*r>>8)&077777u); }
static void dp_divide_input(uint32_t n,uint16_t w[20]) {
    static const uint16_t edge[8]={0,077777,1,077776,037777,040000,020000,057777};
    uint32_t r=n+0x451128u; unsigned i;
    for(i=0;i<20;++i) w[i]=dp_word(&r);
    w[7]=0; w[8]=w[8]|((w[8]&040000u)<<1); w[9]=w[9]|((w[9]&040000u)<<1);
    w[19]=(uint16_t)(n&1u);
    if(n<8192u) { for(i=0;i<4;++i) w[i<2 ? i : i+15]=edge[(n>>(1u+3u*i))&7u]; }
    else if(n<16384u) { w[0]=w[0]&037777u; w[1]=w[1]&037777u; w[17]=w[17]&037777u; w[18]=w[18]&037777u; }
    if(n>=32768u) {
        uint16_t h=(uint16_t)(1u+dp_word(&r)%16383u),mh;
        uint32_t k=n-32768u;
        if((k&15u)<8u) h=(uint16_t)(1u<<(k&7u));
        switch((k>>4)&7u) {
        case 0: mh=0; break;
        case 1: mh=(uint16_t)(h-1u); break;
        case 2: mh=h; break;
        case 3: mh=(uint16_t)(h<16383u ? h+1u:h); break;
        case 4: mh=(uint16_t)(h>=2u?h-2u:0); break;
        default: mh=(uint16_t)(dp_word(&r)%h); break;
        }
        w[0]=mh; w[17]=h;
        if((k>>7)&1u) {w[0]^=077777u;w[1]^=077777u;}
        if((k>>8)&1u) {w[17]^=077777u;w[18]^=077777u;}
    }
}
#endif
