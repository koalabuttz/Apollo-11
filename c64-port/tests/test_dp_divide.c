/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "guidance_dp_divide.h"
#include "dp_divide_cases.h"
#define DP_DIVIDE_FROZEN_IMPLEMENTATION
#include "dp_divide_vectors.h"
static p66_preparation s,before;
static p66_scale_scratch sc;
static p66_elapsed_state el,old_elapsed;
static p66_divide_scratch d;
static unsigned compare_words(const uint16_t *actual,unsigned row) {
    frozen_words r; unsigned i,failures=0; uint16_t word;
    if(!dp_divide_reference_begin(&r,row)) return 1;
    for(i=0;i<18;++i) if(!frozen_words_next(&r,&word) || word!=actual[i]) ++failures;
    if(!frozen_words_finished(&r)) ++failures;
    return failures;
}
unsigned dp_divide_check(void) {
    unsigned row,i,failures=0;
    for(row=0;row<DP_DIVIDE_FROZEN_ROWS;++row) {
        agc_dp operand; uint16_t w[20],actual[18];
        memset(&s,0x25,sizeof s); memset(&el,0x52,sizeof el);
        dp_divide_input(dp_divide_reference_cases[row],w);
        for(i=0;i<8;++i) s.mpac[i]=w[i];
        s.rate.registers.a=w[8];s.rate.registers.l=w[9];
        el.buf=w[10]; d.buf_low=w[11];d.dvsign=w[12];d.dvnormct=w[13];d.maxdvsw=w[14];sc.ovfind=w[15];sc.mptemp=w[16];operand.high=w[17];operand.low=w[18];
        before=s; old_elapsed=el;
        actual[0]=(uint16_t)p66_dp_divide(&s,&sc,&el,&d,operand,(uint8_t)w[19]);
        for(i=0;i<8;++i) actual[i+1]=s.mpac[i];
        actual[9]=s.rate.registers.a;actual[10]=s.rate.registers.l;actual[11]=el.buf;actual[12]=d.buf_low;actual[13]=d.dvsign;actual[14]=d.dvnormct;actual[15]=d.maxdvsw;actual[16]=sc.ovfind;actual[17]=sc.mptemp;
        failures+=compare_words(actual,row);
        for(i=0;i<3;++i) before.mpac[i]=s.mpac[i];
        before.rate.registers=s.rate.registers;old_elapsed.buf=el.buf;
        if(memcmp(&s,&before,sizeof s) || memcmp(&el,&old_elapsed,sizeof el)) ++failures;
    }
    return failures;
}
#ifndef DP_DIVIDE_CHECK_ONLY
int main(void) {
    unsigned failures=dp_divide_check(); uint32_t mode;
    agc_dp operand={1,0}; p66_scale_scratch old_scale; p66_divide_scratch old_divide;
    memset(&s,0x55,sizeof s);memset(&sc,0x35,sizeof sc);memset(&el,0x53,sizeof el);memset(&d,0x33,sizeof d);
    old_scale=sc;old_elapsed=el;old_divide=d;
    for(mode=1;mode<=077777u;++mode) {
        s.mpac[7]=(uint16_t)mode; before=s;
        if(p66_dp_divide(&s,&sc,&el,&d,operand,0)!=P66_DIVIDE_OUT_OF_SCOPE ||
           memcmp(&s,&before,sizeof s) || memcmp(&sc,&old_scale,sizeof sc) ||
           memcmp(&el,&old_elapsed,sizeof el) || memcmp(&d,&old_divide,sizeof d)) ++failures;
    }
    s.mpac[7]=0; before=s;
    operand.high=0100000; operand.low=0;
    if(p66_dp_divide(&s,&sc,&el,&d,operand,0)!=P66_DIVIDE_OUT_OF_SCOPE ||
       memcmp(&s,&before,sizeof s) || memcmp(&sc,&old_scale,sizeof sc) ||
       memcmp(&el,&old_elapsed,sizeof el) || memcmp(&d,&old_divide,sizeof d)) ++failures;
    operand.high=0; operand.low=0100000;
    if(p66_dp_divide(&s,&sc,&el,&d,operand,1)!=P66_DIVIDE_OUT_OF_SCOPE ||
       memcmp(&s,&before,sizeof s) || memcmp(&sc,&old_scale,sizeof sc) ||
       memcmp(&el,&old_elapsed,sizeof el) || memcmp(&d,&old_divide,sizeof d)) ++failures;
    printf("%u complete DDV/BDDV states, untouched retained memory, 32767 MODE / 2 operand rejections: %s\n",DP_DIVIDE_FROZEN_ROWS,failures?"FAIL":"PASS");
    return failures?1:0;
}
#endif
