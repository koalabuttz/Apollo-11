/* SPDX-License-Identifier: MIT */
#include "dsky_scalar_output.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,agc_word address)
{
    agc_dp d;d.high=w[address];d.low=w[address+1u];agc_double_add(r,&d);w[address]=d.high;w[address+1u]=d.low;
}
/* Source TP SHORTMP: partial-product stores and DAS carries remain unrounded. */
static void short_product(uint16_t *w,agc_word multiplier)
{
    agc_word middle=w[0155],high=w[0154];agc_register_pair r;
    w[0135]=multiplier;r=agc_multiply(agc_extend(multiplier),w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;
    r=agc_multiply(agc_extend(middle),multiplier);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);
    r=agc_multiply(agc_extend(high),multiplier);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;
}
static void exchange_pair(uint16_t *w,agc_word address)
{
    agc_word high=w[address],low=w[address+1u];w[address]=agc_correct(w[0]);w[address+1u]=agc_correct(w[1]);w[0]=agc_extend(high);w[1]=agc_extend(low);
}
static void degrees(uint16_t *w,agc_word index)
{
    static const agc_word factors[4]={05605,03656,016314,031463};agc_word angle=w[0154];agc_register_pair r;
    w[0156]=index;
    if(angle&AGC_SIGN){w[0154]=(agc_word)(angle&037777u);w[0123]=factors[index];w[0124]=factors[index+1u];}
    w[0]=agc_extend(factors[index]);w[1]=agc_extend(factors[index+1u]);exchange_pair(w,0154);
    short_product(w,agc_correct(w[0]));exchange_pair(w,0123);r.a=w[0];r.l=w[1];accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;
}
enum p66_bias_status dsky_scalar_output(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_scalar_output_entry entry,agc_word channel33)
{
    static const agc_word entry_pc[2][6]={{02615,02677,02704,02623,02635,0},{02516,02603,02610,02524,0,02653}};
    uint16_t *w;agc_word address=0,eb,logical,multiplier,old;agc_register a;agc_register_pair r;
    if(s==0||io==0||(unsigned)mission>1u||(unsigned)entry>5u||entry_pc[(unsigned)mission][(unsigned)entry]==0)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;eb=(agc_word)(w[3]>>8);
    if(w[5]!=entry_pc[(unsigned)mission][(unsigned)entry]||w[4]!=060000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(060000u|eb)||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||(entry==DSKY_SCALAR_LANDING_POSITION&&channel33>AGC_MASK))return P66_BIAS_OUT_OF_SCOPE;
    if(entry==DSKY_SCALAR_FRACTION){
        if(w[0140]!=1u&&w[0140]!=2u)return P66_BIAS_OUT_OF_SCOPE;
        if(w[0140]==2u){if(w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;address=(agc_word)(w[0150u+w[0117]]&03777u);eb=(agc_word)(address>>8);}
        else {logical=w[0145];if(logical>01777u)return P66_BIAS_OUT_OF_SCOPE;address=logical<01400u?logical:(agc_word)(w[3]+(logical&0377u));}
        if(address<01000u||address>03776u||(address&0377u)==0377u)return P66_BIAS_OUT_OF_SCOPE;
    }
    w[1]=agc_extend(agc_correct(w[1]));
    if(entry==DSKY_SCALAR_FRACTION){w[0144]=02654;w[1]=agc_extend(w[0155]);w[0154]=w[address];w[0155]=w[address+1u];w[0156]=0;w[0]=0;w[3]=(agc_word)(eb*0400u);}
    else if(entry==DSKY_SCALAR_DEGREES)degrees(w,0);
    else if(entry==DSKY_SCALAR_MISSION_ANGLE){
        if(mission==DSKY_NV_LM){if(w[0154]&037777u)w[0154]=agc_correct(agc_add_register(agc_extend(agc_negate(w[0154])),1));degrees(w,0);}
        else {
            old=w[0154];a=agc_extend(old);
            if(old==AGC_NEG_ZERO)a=agc_extend(077776u);
            a=agc_add_register(a,agc_extend(016040));
            if((old&AGC_SIGN)&&old!=AGC_NEG_ZERO&&agc_correct(a)!=AGC_NEG_ZERO&&agc_correct(a)>0u&&agc_correct(a)<AGC_SIGN)a=(agc_register)(a-1u);
            w[0154]=agc_correct(a);
            if(agc_overflow(a))w[0154]=agc_correct(agc_add_register(agc_extend(AGC_SIGN),agc_extend(w[0154])));
            degrees(w,2);
        }
    }
    else {
        if(entry==DSKY_SCALAR_LANDING_POSITION){r=agc_multiply(agc_extend(channel33),01000);w[0154]=(agc_word)(agc_negate(agc_correct(r.a))&3u);w[0]=w[0154];w[1]=r.l;}
        exchange_pair(w,0123);exchange_pair(w,0154);multiplier=agc_correct(w[0]);
        if(multiplier==AGC_NEG_ZERO){w[0135]=multiplier;w[0154]=w[0155]=w[0156]=AGC_NEG_ZERO;w[0]=agc_extend(AGC_NEG_ZERO);}
        else short_product(w,multiplier);
        if(entry==DSKY_SCALAR_ARITHMETIC14){old=w[0156];w[0156]=agc_correct(w[0]);w[0]=agc_extend(w[0155]);w[0155]=old;w[0154]=agc_correct(w[0]);}
    }
    w[2]=00566;w[4]=062000;w[5]=02566;w[6]=(agc_word)(062000u|eb);return P66_BIAS_OK;
}
