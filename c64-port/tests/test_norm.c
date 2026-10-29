/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "guidance_norm.h"
#include "norm_cases.h"
#define NORM_FROZEN_IMPLEMENTATION
#include "norm_vectors.h"
static p66_preparation s,old_s;
static p66_scale_scratch sc,old_sc;
static p66_elapsed_state el,old_el;
static p66_divide_scratch d,old_d;
static p66_norm_scratch n,old_n;
unsigned norm_check(void){
    unsigned row,i,failures=0;
    for(row=0;row<NORM_FROZEN_ROWS;++row){
        uint32_t index=norm_reference_cases[row];uint16_t w[20],actual[21],expected;frozen_words r;
        memset(&s,0x25,sizeof s);memset(&el,0x52,sizeof el);memset(&n,0x13,sizeof n);norm_input(index,w);
        for(i=0;i<8;++i)s.mpac[i]=w[i];
        s.rate.registers.a=w[8];s.rate.registers.l=w[9];sc.mptemp=w[10];sc.ovfind=w[11];el.buf=w[12];d.buf_low=w[13];d.dvsign=w[14];d.dvnormct=w[15];d.maxdvsw=w[16];n.sr=w[17];n.cyl=w[18];n.addrwd=w[19];
        old_s=s;old_sc=sc;old_el=el;old_d=d;old_n=n;
        actual[0]=(uint16_t)((index&1u)?p66_normalized_square_root(&s,&sc,&el,&d,&n):p66_square_root(&s,&sc,&el,&d,&n));
        for(i=0;i<8;++i)actual[i+1]=s.mpac[i];
        actual[9]=s.rate.registers.a;actual[10]=s.rate.registers.l;actual[11]=sc.mptemp;actual[12]=sc.ovfind;actual[13]=el.buf;actual[14]=d.buf_low;actual[15]=d.dvsign;actual[16]=d.dvnormct;actual[17]=d.maxdvsw;actual[18]=n.sr;actual[19]=n.cyl;actual[20]=n.addrwd;
        if(!norm_reference_begin(&r,row))++failures;
        else{for(i=0;i<21;++i)if(!frozen_words_next(&r,&expected)||expected!=actual[i])++failures;if(!frozen_words_finished(&r))++failures;}
        for(i=0;i<3;++i)old_s.mpac[i]=s.mpac[i];
        old_s.rate.registers=s.rate.registers;old_sc.mptemp=sc.mptemp;old_el.buf=el.buf;old_d.buf_low=d.buf_low;old_n.sr=n.sr;old_n.cyl=n.cyl;old_n.addrwd=n.addrwd;
        if(memcmp(&s,&old_s,sizeof s)||memcmp(&sc,&old_sc,sizeof sc)||memcmp(&el,&old_el,sizeof el)||memcmp(&d,&old_d,sizeof d)||memcmp(&n,&old_n,sizeof n))++failures;
    }return failures;
}
#ifndef NORM_CHECK_ONLY
int main(void){
    unsigned failures=norm_check();uint32_t mode;
    memset(&s,0x25,sizeof s);memset(&sc,0x15,sizeof sc);memset(&el,0x52,sizeof el);memset(&d,0x24,sizeof d);memset(&n,0x13,sizeof n);
    old_sc=sc;old_el=el;old_d=d;old_n=n;
    for(mode=1;mode<=077777u;++mode){
        s.mpac[7]=(agc_word)mode;old_s=s;
        if(p66_square_root(&s,&sc,&el,&d,&n)!=P66_NORM_OUT_OF_SCOPE)++failures;
        if(mode!=077776u&&p66_normalized_square_root(&s,&sc,&el,&d,&n)!=P66_NORM_OUT_OF_SCOPE)++failures;
        if(memcmp(&s,&old_s,sizeof s)||memcmp(&sc,&old_sc,sizeof sc)||memcmp(&el,&old_el,sizeof el)||memcmp(&d,&old_d,sizeof d)||memcmp(&n,&old_n,sizeof n))++failures;
    }
    printf("%u SQRT/SQRTSUB states, retained state and unsupported MODE: %s\n",NORM_FROZEN_ROWS,failures?"FAIL":"PASS");return failures?1:0;
}
#endif
