/* SPDX-License-Identifier: MIT */
#include "dsky_nv_admission.h"
static uint8_t available_argument(agc_word a)
{
    return (uint8_t)(a<AGC_SIGN||a>=077773u);
}
static void second(uint16_t *w,enum dsky_nv_route *route)
{
    if(w[0160]==0){w[0]=agc_extend(w[0157]);w[2]=03575;w[5]=02317;*route=DSKY_NV_NOUN_ONLY;}
    else{w[0161]=w[0156];w[0]=agc_extend(w[0160]);w[2]=03602;w[5]=02337;*route=DSKY_NV_VERB_NOUN;}
}
enum p66_bias_status dsky_nv_admit(apollo_overlay_state *s,p66_bias_io *io,
 enum dsky_nv_mission mission,enum dsky_nv_entry entry,enum dsky_nv_route *route)
{
    uint16_t *w;agc_word a,caller,busy,old_options;agc_word start,end;
    if(s==0||io==0||route==0||
       (mission!=DSKY_NV_LM&&mission!=DSKY_NV_CM)||(unsigned)entry>(unsigned)DSKY_NV_NVSUB2)
        return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;start=mission==DSKY_NV_LM?04154u:04170u;
    end=mission==DSKY_NV_LM?04202u:04216u;
    if(w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||
       w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->superbank>0160u||
       (io->superbank&017u)||io->interrupt_enabled>1u)
        return P66_BIAS_OUT_OF_SCOPE;
    if(entry<=DSKY_NV_NVMONOPT){
        if(w[5]!=(agc_word)(start+(entry==DSKY_NV_NVMONOPT?1u:0u))||
           w[2]<02000u||w[2]>07777u||w[01012]>7u||agc_overflow(w[0]))
            return P66_BIAS_OUT_OF_SCOPE;
        a=agc_correct(w[0]);busy=(agc_word)((w[01021]&020000u)+w[01012]);
        if(!busy&&!available_argument(a))return P66_BIAS_OUT_OF_SCOPE;
        caller=w[2];w[1]=agc_extend(agc_correct(w[1]));
        if(entry==DSKY_NV_NVSUB)w[1]=0;
        w[0123]=a;
        if(busy){w[0]=(agc_word)(busy-1u);w[2]=3;w[5]=caller;*route=DSKY_NV_BLOCKED;return P66_BIAS_OK;}
        w[01037]=agc_add_word(caller,1);old_options=w[01022];
        w[01022]=agc_correct(w[1]);w[1]=agc_extend(old_options);
        w[01021]=040000;w[01040]=(agc_word)(w[6]|io->superbank);
        w[3]=0400;w[4]=062000;w[6]=062001;io->superbank=0100;
    }else{
        if(w[5]!=(entry==DSKY_NV_NVSUB1?03544u:03571u)||w[4]!=062000u||
           !(io->superbank&0100u))return P66_BIAS_OUT_OF_SCOPE;
        if(entry==DSKY_NV_NVSUB1&&(w[0123]>AGC_MASK||!available_argument(w[0123])))return P66_BIAS_OUT_OF_SCOPE;
        if(entry==DSKY_NV_NVSUB2&&(w[0157]>0177u||w[0160]>0177u))return P66_BIAS_OUT_OF_SCOPE;
        w[1]=agc_extend(agc_correct(w[1]));
    }
    if(entry==DSKY_NV_NVSUB2){second(w,route);return P66_BIAS_OK;}
    w[0136]=end;a=w[0123];w[0]=agc_diminished_abs(a);
    if(a==0||a==AGC_NEG_ZERO){w[2]=a==0?03551u:03553u;w[5]=02351;*route=DSKY_NV_ALARM;return P66_BIAS_OK;}
    if(a&AGC_SIGN){w[2]=03552;w[5]=03516;*route=DSKY_NV_BLANK;return P66_BIAS_OK;}
    w[0157]=(agc_word)(a&0177u);w[0160]=(agc_word)((a>>7)&0177u);w[023]=0;
    if(w[0157]==0){w[0]=agc_extend(w[0160]);w[2]=03566;w[5]=02337;*route=DSKY_NV_VERB_ONLY;return P66_BIAS_OK;}
    second(w,route);return P66_BIAS_OK;
}
