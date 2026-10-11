/* SPDX-License-Identifier: MIT */
#ifndef NORM_CASES_H
#define NORM_CASES_H
#include <stdint.h>
#define NORM_CASES 132096u
static uint16_t norm_random(uint32_t *r){*r=*r*1664525u+1013904223u;return (uint16_t)((*r>>8)&077777u);}
static void norm_input(uint32_t index,uint16_t w[20]){
    static const uint16_t edge[8]={0,077777,1,077776,010000,020000,037777,040000};
    uint32_t r=(index>>1)+0x314159u;unsigned i;
    for(i=0;i<20;++i)w[i]=norm_random(&r);
    w[7]=(index&1u)&&(index&2u)?077776:0;w[8]=(uint16_t)(w[8]|((w[8]&040000u)<<1));w[9]=(uint16_t)(w[9]|((w[9]&040000u)<<1));
    index>>=1;
    if(index<512u)for(i=0;i<3;++i)w[i]=edge[(index>>(3u*i))&7u];
    else w[0]=(uint16_t)((index-512u)&077777u);
}
#endif
