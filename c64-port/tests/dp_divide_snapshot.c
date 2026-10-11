/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "guidance_dp_divide.h"
#include "dp_divide_cases.h"
int main(void) {
    uint32_t n; for(n=0;n<DP_DIVIDE_CASES;++n) {
        p66_preparation s; p66_scale_scratch sc; p66_elapsed_state el; p66_divide_scratch d;
        agc_dp operand; uint16_t w[20]; unsigned i,path;
        memset(&s,0,sizeof s); memset(&el,0,sizeof el); dp_divide_input(n,w);
        for(i=0;i<8;++i) s.mpac[i]=w[i];
        s.rate.registers.a=w[8]; s.rate.registers.l=w[9];
        el.buf=w[10]; d.buf_low=w[11]; d.dvsign=w[12]; d.dvnormct=w[13]; d.maxdvsw=w[14]; sc.ovfind=w[15]; sc.mptemp=w[16]; operand.high=w[17]; operand.low=w[18];
        path=p66_dp_divide(&s,&sc,&el,&d,operand,(uint8_t)w[19]);
        printf("%u %u",(unsigned)n,path); for(i=0;i<8;++i) printf(" %u",s.mpac[i]);
        printf(" %u %u %u %u %u %u %u %u %u\n",s.rate.registers.a,s.rate.registers.l,el.buf,d.buf_low,d.dvsign,d.dvnormct,d.maxdvsw,sc.ovfind,sc.mptemp);
    } return 0;
}
