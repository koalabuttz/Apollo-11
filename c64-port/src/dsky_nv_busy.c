/* SPDX-License-Identifier: MIT */
#include "dsky_nv_busy.h"
static uint8_t abort_check(uint16_t *w)
{
    agc_word c=w[01042];
    w[0]=agc_diminished_abs(c);
    if(c!=0&&c!=AGC_NEG_ZERO)return 1;
    w[0]=agc_diminished_abs(w[01043]);
    return (uint8_t)(w[01043]!=0);
}
enum p66_bias_status dsky_nv_busy(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,enum dsky_nv_mission mission,enum dsky_nv_busy_entry entry,enum dsky_nv_busy_route *route)
{
    uint16_t *w;agc_word start,sy,sleep,abort,poo,wait,pre,busy,free_entry,q;agc_register a,l;
    if(s==0||io==0||channel11==0||route==0||(mission!=DSKY_NV_LM&&mission!=DSKY_NV_CM)||(unsigned)entry>(unsigned)DSKY_NV_BUSY_INNER)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;start=mission==DSKY_NV_LM?04207u:04223u;sy=mission==DSKY_NV_LM?02647u:02577u;sleep=mission==DSKY_NV_LM?05133u:05175u;abort=mission==DSKY_NV_LM?04227u:04243u;poo=mission==DSKY_NV_LM?05652u:05721u;pre=mission==DSKY_NV_LM?04437u:04453u;busy=(agc_word)(pre+3u);wait=(agc_word)(pre+6u);free_entry=mission==DSKY_NV_LM?04164u:04200u;
    if(w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||w[01043]==AGC_NEG_ZERO||w[01043]>AGC_MASK||w[01042]>AGC_MASK||w[2]<02000u||w[2]>03777u||agc_overflow(w[0]))return P66_BIAS_OUT_OF_SCOPE;
    if(w[5]!=(entry==DSKY_NV_ENDIDLE?start:entry==DSKY_NV_BUSY?busy:entry==DSKY_NV_PREBUSY?pre:entry==DSKY_NV_WAIT?wait:sy)||(entry==DSKY_NV_BUSY_INNER&&w[4]!=010000u)||(entry==DSKY_NV_WAIT&&w[01012]>7u))return P66_BIAS_OUT_OF_SCOPE;
    a=w[0];l=w[1];q=w[2];w[1]=agc_extend(agc_correct(l));
    if(entry==DSKY_NV_WAIT){w[1]=0;w[0123]=agc_correct(a);w[0]=agc_extend((agc_word)((w[01021]&020000u)+w[01012]));if(w[0]==0){w[5]=free_entry;*route=DSKY_NV_AVAILABLE_ENTRY;return P66_BIAS_OK;}w[2]=agc_add_word(q,1);q=w[2];entry=DSKY_NV_PREBUSY;}
    if(entry==DSKY_NV_PREBUSY){a=agc_add_register(agc_extend((agc_word)(AGC_MASK^02003u)),agc_extend(q));a=agc_add_register(a,agc_extend(w[4]));}
    if(entry==DSKY_NV_BUSY||entry==DSKY_NV_PREBUSY){w[4]=010000;w[6]=(agc_word)(010000u|(w[3]>>8));w[2]=(agc_word)(sy&01777u);}
    if(entry==DSKY_NV_ENDIDLE){w[1]=agc_extend(q);w[2]=agc_extend(agc_correct(l));}
    else w[1]=agc_extend(agc_correct(a));
    if(abort_check(w)){w[2]=(agc_word)(abort+1u);w[5]=poo;*route=DSKY_NV_ABORT_ENTRY;return P66_BIAS_OK;}
    if(entry==DSKY_NV_ENDIDLE){w[0]=agc_add_register(agc_extend((agc_word)(agc_correct(w[1])&01777u)),agc_extend(w[4]));w[01042]=agc_correct(w[0]);w[2]=(agc_word)(start+010u);}
    else{*channel11|=020u;w[0]=w[1];w[01043]=agc_correct(w[0]);w[2]=(agc_word)(sy+7u);}
    w[5]=sleep;*route=DSKY_NV_SLEEP_ENTRY;return P66_BIAS_OK;
}
