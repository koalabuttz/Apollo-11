/* SPDX-License-Identifier: MIT */
#include "throttle_service_cases.h"
static agc_word next(uint32_t *seed){*seed=*seed*UINT32_C(1664525)+UINT32_C(1013904223);return (agc_word)((*seed>>8)&AGC_MASK);}
void ts_input(throttle_service_case *c,uint16_t n){
    static const agc_word edge[8]={0,077777,1,077776,020000,057777,037777,040000};
    p66_preparation *s=&c->previous.previous.base.previous.previous.previous.preparation;
    uint32_t seed=UINT32_C(0x31f2221)+(uint32_t)n;uint8_t i;agc_word v[23];
    tf_input(&c->previous,n);s->mpac[7]=(agc_word)(n<8192u?0u:(n&1u?077777u:((n*17u)&AGC_MASK)));for(i=0;i<23u;++i)v[i]=next(&seed);
    c->service.fcodd.high=v[0];c->service.fcodd.low=v[1];c->service.fp.high=v[2];c->service.fp.low=v[3];c->service.pif.high=v[4];c->service.pif.low=v[5];
    c->service.fc=v[6];c->service.fcold=v[7];c->service.pifpset=v[8];c->service.pseudo55=v[9];c->service.tthrot=v[10];c->service.rtnhold=v[11];c->service.thrust=v[12];c->service.channel14=v[13];
    c->service.lowcrit=v[14];c->service.highcrit=v[15];c->service.modreg=(agc_word)((n&3u)?0102:v[16]);c->service.phase3[0]=v[17];c->service.phase3[1]=v[18];c->service.q=(agc_word)(0100u+(v[19]%07600u));c->service.bbank=062007;c->service.superbank=(agc_word)((n&3u)<<4);c->service.interrupt_enabled=(agc_word)(n&1u);
    if(n<4096u){
        s->rate.registers.a=agc_extend(edge[(n>>3)&7u]);s->rate.registers.l=agc_extend(edge[(n>>6)&7u]);
        c->previous.feedback.afc.high=edge[(n>>9)&7u];c->previous.feedback.afc.low=edge[n&7u];
        c->previous.input.mass.high=edge[(n>>4)&7u];c->previous.input.mass.low=edge[(n>>7)&7u];
        c->service.lowcrit=0700;c->service.highcrit=03500;c->service.fcold=edge[(n>>5)&7u];
    }else if(n<8192u){
        s->rate.registers.a=0;s->rate.registers.l=next(&seed);c->previous.feedback.afc.high=0;c->previous.feedback.afc.low=next(&seed);
        c->previous.input.mass.high=04000;c->previous.input.mass.low=0;c->service.lowcrit=0700;c->service.highcrit=03500;
        c->service.fcold=(agc_word)((n&1u)?03477:03501);
        c->service.tthrot=(agc_word)(s->time.low&037777u);c->previous.feedback.fweight.high=0;c->previous.feedback.fweight.low=0;
    }
    if(n>=8192u){s->rate.registers.a=(agc_register)(next(&seed)|((n&1u)?0100000u:0u));s->rate.registers.l=(agc_register)(next(&seed)|((n&2u)?0100000u:0u));}
    /* Process+lag spans0..255; this explicitly exercises hardwareDV overflow. */
    s->this_piptime.low=(agc_word)(((uint32_t)s->time.low+024u+077777u-(n&0377u))&AGC_MASK);
}
void ts_words(const throttle_service_case *c,uint16_t w[TS_WORDS]){
    tf_words(&c->previous,w);w[124]=c->service.fcodd.high;w[125]=c->service.fcodd.low;w[126]=c->service.fp.high;w[127]=c->service.fp.low;w[128]=c->service.pif.high;w[129]=c->service.pif.low;
    w[130]=c->service.fc;w[131]=c->service.fcold;w[132]=c->service.pifpset;w[133]=c->service.pseudo55;w[134]=c->service.tthrot;w[135]=c->service.rtnhold;w[136]=c->service.thrust;w[137]=c->service.channel14;
    w[138]=c->service.lowcrit;w[139]=c->service.highcrit;w[140]=c->service.modreg;w[141]=c->service.phase3[0];w[142]=c->service.phase3[1];w[143]=c->service.q;w[144]=c->service.bbank;w[145]=c->service.superbank;w[146]=c->service.interrupt_enabled;
}
#ifndef THROTTLE_SERVICE_INPUT_ONLY
unsigned ts_apply(throttle_service_case *c){return p66_throttle_service(&c->previous.previous.base.previous.previous.previous.preparation,&c->previous.previous.base.previous.previous.previous.scratch,&c->previous.previous.base.previous.elapsed,&c->previous.previous.base.scratch,&c->previous.previous.norm,&c->previous.feedback,&c->previous.input,&c->service);}
#endif
