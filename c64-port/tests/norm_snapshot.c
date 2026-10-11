/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "guidance_norm.h"
#include "norm_cases.h"
int main(void){uint32_t index;for(index=0;index<NORM_CASES;++index){
    p66_preparation s;p66_scale_scratch sc;p66_elapsed_state el;p66_divide_scratch d;p66_norm_scratch n;
    uint16_t w[20];unsigned i,path;memset(&s,0,sizeof s);memset(&el,0,sizeof el);memset(&n,0,sizeof n);norm_input(index,w);
    for(i=0;i<8;++i)s.mpac[i]=w[i];
    s.rate.registers.a=w[8];s.rate.registers.l=w[9];sc.mptemp=w[10];sc.ovfind=w[11];el.buf=w[12];d.buf_low=w[13];d.dvsign=w[14];d.dvnormct=w[15];d.maxdvsw=w[16];n.sr=w[17];n.cyl=w[18];n.addrwd=w[19];
    path=(unsigned)((index&1u)?p66_normalized_square_root(&s,&sc,&el,&d,&n):p66_square_root(&s,&sc,&el,&d,&n));printf("%u %u",(unsigned)index,path);
    for(i=0;i<8;++i)printf(" %u",s.mpac[i]);
    printf(" %u %u %u %u %u %u %u %u %u %u %u %u\n",s.rate.registers.a,s.rate.registers.l,sc.mptemp,sc.ovfind,el.buf,d.buf_low,d.dvsign,d.dvnormct,d.maxdvsw,n.sr,n.cyl,n.addrwd);
}return 0;}
