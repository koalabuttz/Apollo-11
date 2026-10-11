/* SPDX-License-Identifier: MIT */
#include "dsky_nv_continuation.h"
enum p66_bias_status dsky_nv_continue(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_nv_continuation_entry entry)
{
    static const agc_word pc[2][9]={{04164,04170,04204,03570,03566,03575,03602,03604,03612},{04200,04204,04220,03570,03566,03575,03602,03604,03612}};
    uint16_t *w;agc_word options;agc_register a;
    if(s==0||io==0||(unsigned)mission>1u||(unsigned)entry>8u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;
    if(w[5]!=pc[mission][entry]||w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
    if(entry>=DSKY_NV_ENTSET&&(w[4]!=062000u||!(io->superbank&0100u)))return P66_BIAS_OUT_OF_SCOPE;
    if(entry==DSKY_NV_COMMON&&(w[0]>1u||w[2]<02000u||w[2]>07776u))return P66_BIAS_OUT_OF_SCOPE;
    if(entry==DSKY_NV_KILL&&(w[2]<02000u||w[2]>07777u))return P66_BIAS_OUT_OF_SCOPE;
    if(entry>=DSKY_NV_ENTSET&&entry<=DSKY_NV_NOUN_RETURN&&(w[01037]<02000u||w[01037]>07777u||(w[01040]&~076167u)))return P66_BIAS_OUT_OF_SCOPE;
    w[1]=agc_extend(agc_correct(w[1]));
    if(entry==DSKY_NV_COMMON){a=agc_add_register(w[0],agc_extend(w[2]));w[01037]=agc_correct(a);options=w[01022];w[01022]=agc_correct(w[1]);w[1]=agc_extend(options);w[01021]=040000;}
    if(entry==DSKY_NV_COMMON||entry==DSKY_NV_SWITCH){w[01040]=(agc_word)(w[6]|io->superbank);w[3]=0400;w[4]=062000;w[6]=062001;io->superbank=0100;w[0]=agc_extend(062101);w[2]=02001;w[5]=03544;return P66_BIAS_OK;}
    if(entry==DSKY_NV_KILL){w[01021]=040000;w[0]=agc_extend(040000);w[5]=w[2];w[2]=3;return P66_BIAS_OK;}
    if(entry<=DSKY_NV_NOUN_RETURN){if(entry==DSKY_NV_ENTSET_CLEAR){w[0]=0;w[01013]=0;}w[5]=mission==DSKY_NV_LM?04202u:04216u;return dsky_nv_return(s,io,mission);}
    if(entry==DSKY_NV_AFTER_VERB){w[0]=agc_extend(w[0157]);w[2]=03604;w[5]=02317;return P66_BIAS_OK;}
    if(entry==DSKY_NV_AFTER_NOUN){w[01014]=w[01015]=w[01013]=0;w[0156]=w[0161];w[0]=agc_extend(w[0161]);}
    w[2]=03613;w[5]=02035;return P66_BIAS_OK;
}
