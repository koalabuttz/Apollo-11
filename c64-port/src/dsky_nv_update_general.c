/* SPDX-License-Identifier: MIT */
#include "dsky_nv_update_general.h"
#include "agc_multiword.h"
#include "dsky_pinball.h"
#include "noun_reader.h"
static void wide_accumulate(agc_register_pair *r,uint16_t *w,uint16_t address)
{
    agc_dp d;d.high=w[address];d.low=w[address+1u];agc_double_add(r,&d);w[address]=d.high;w[address+1u]=d.low;
}
static void wide_shortmp(uint16_t *w)
{
    agc_register_pair r;agc_word minor=w[0155],major=w[0154];
    w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;
    r=agc_multiply(agc_extend(minor),10);wide_accumulate(&r,w,0155);w[0154]=agc_correct(r.a);
    r=agc_multiply(agc_extend(major),10);wide_accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;
}
static enum dsky_nv_update_route wide_decimal(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{
    static const agc_word codes[10]={025,003,031,033,017,036,034,023,035,037};
    uint16_t *w=s->words;agc_word caller=w[2],count=w[0777],relative;uint8_t i;agc_register_pair r;p66_bias_io arithmetic_io;
    r=agc_multiply(w[0],0244);w[1]=agc_extend(w[0154]);w[0154]=agc_correct(r.l);w[0155]=0;w[0115]=caller;
    for(i=0;i<2u;++i){
        w[0137]=(uint16_t)(1u-i);wide_shortmp(w);w[0124]=codes[w[0154]==AGC_NEG_ZERO?0u:w[0154]];
        w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0143]=(agc_word)(count-i);w[0777]=(agc_word)(count-i-1u);
        w[2]=mission==DSKY_NV_LM?03253u:03160u;relative=(agc_word)((count-i)>>1);
        if(w[01023u+relative]==0){
            w[0114]=w[2];w[021]=relative;w[0141]=relative;
            if((w[0143]&1u)!=0){w[0124]=(agc_word)(w[0124]<<5);w[0143]=1;}else w[0143]=0;
            w[0]=0;w[2]=mission==DSKY_NV_LM?03354u:03261u;w[5]=mission==DSKY_NV_LM?05705u:05747u;io->interrupt_enabled=0;
            return DSKY_NV_UPDATE_CCSHOLE;
        }
        /* The arithmetic helper has no SUPER writes. The independent wider
         * source proof qualifies its relay math for each selecting SUPER. */
        arithmetic_io.superbank=0100;arithmetic_io.interrupt_enabled=io->interrupt_enabled;
        (void)dsky_pinball_write(s,&arithmetic_io,DSKY_DSPIN);io->interrupt_enabled=arithmetic_io.interrupt_enabled;
    }
    w[0]=0177754;w[2]=0116;w[5]=caller;w[0777]=077754;w[7]=0;return DSKY_NV_UPDATED;
}
enum p66_bias_status dsky_nv_update_general(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_nv_update_entry entry,enum dsky_nv_update_route *route)
{
    static const agc_word pc[4]={02337,02340,02317,02320};
    uint16_t *w;agc_word value,caller,oldbank;uint8_t noun;apollo_noun_reader_io table_io;
    if(s==0||io==0||route==0||(unsigned)mission>1u||(unsigned)entry>3u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;noun=(uint8_t)(entry>=DSKY_NV_NOUN_STORE);value=entry==DSKY_NV_VERB_STORE||entry==DSKY_NV_NOUN_STORE?agc_correct(w[0]):w[noun?01002u:01001u];
    if(w[5]!=pc[entry]||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||(value>99u&&value!=AGC_NEG_ZERO)||w[2]<02000u||w[2]>07777u||((entry==DSKY_NV_VERB_STORE||entry==DSKY_NV_NOUN_STORE)&&agc_overflow(w[0])))return P66_BIAS_OUT_OF_SCOPE;
    caller=w[2];oldbank=w[6];w[1]=agc_extend(agc_correct(w[1]));w[noun?01002u:01001u]=value;w[0117]=caller;
    if(noun){w[0]=02325;w[1]=agc_extend(oldbank);w[3]=0400;w[4]=064000;w[6]=064001;w[5]=mission==DSKY_NV_LM?02103u:02062u;table_io.superbank=io->superbank;table_io.interrupt_enabled=io->interrupt_enabled;if(!apollo_noun_reader(s,&table_io,mission==DSKY_NV_LM?APOLLO_FIXED_LM:APOLLO_FIXED_CM,APOLLO_LODNNTAB))return P66_BIAS_OUT_OF_SCOPE;if(w[0146]<AGC_SIGN){w[01017]=w[0146];w[3]=(agc_word)(w[0146]&03400u);w[0145]=(agc_word)(01400u+(w[0146]&0377u));}}
    w[0777]=noun?021u:023u;w[0]=agc_extend(value);w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[5]=mission==DSKY_NV_LM?03306u:03213u;w[2]=mission==DSKY_NV_LM?03320u:03225u;
    *route=wide_decimal(s,io,mission);if(*route==DSKY_NV_UPDATE_CCSHOLE)return P66_BIAS_OK;
    w[4]=062000;w[6]=(agc_word)(062000u|(w[3]>>8));w[2]=0120;w[5]=caller;return P66_BIAS_OK;
}
