/* SPDX-License-Identifier: MIT */
#include "dsky_two_integer.h"
#include "dsky_five_blank.h"
#include "dsky_read_pair.h"
#include "agc_multiword.h"
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==2 || DSKY_TWO_INTEGER_ONLY_STAGE==3
static void sum_pair(agc_register_pair *r,uint16_t *w,uint16_t at)
{agc_dp d;d.high=w[at];d.low=w[at+1u];agc_double_add(r,&d);w[at]=d.high;w[at+1u]=d.low;}
static void short_product(uint16_t *w)
{agc_register_pair r;agc_word lo=w[0155],hi=w[0154];w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(lo),10);sum_pair(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(hi),10);sum_pair(&r,w,0154);w[0]=r.a;w[1]=r.l;}
/* Source DSPDECVN also serves even data positions in 2INTOUT. The older
 * public two-digit kernel is qualified for odd verb/noun positions only. */
static void data_digits(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{static const agc_word digit[]={025,003,031,033,017,036,034,023,035,037};uint16_t *w=s->words;agc_word caller=w[2],count=w[0777],code;agc_register_pair r;unsigned i;p66_bias_io physical;
 physical.superbank=0100;physical.interrupt_enabled=io->interrupt_enabled;r=agc_multiply(w[0],0244);w[1]=agc_extend(w[0154]);w[0154]=agc_correct(r.l);w[0155]=0;w[0115]=caller;
 for(i=0;i<2u;++i){w[0137]=(agc_word)(1u-i);short_product(w);code=digit[w[0154]==077777u?0u:w[0154]];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0124]=code;w[0143]=(agc_word)(count-i);w[0777]=(agc_word)(count-i-1u);w[2]=03253;(void)dsky_pinball_write(s,&physical,DSKY_DSPIN);}
 if(mission==DSKY_NV_CM)w[0114]=03160;
 io->interrupt_enabled=physical.interrupt_enabled;w[0]=0177754;w[2]=0116;w[5]=caller;w[0777]=077754;w[7]=0;
}
#endif
enum p66_bias_status dsky_two_integer(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_two_integer_stage stage)
{
 static const agc_word lm[]={02732,02733,02734,02736,02745},cm[]={02636,02637,02640,02642,02651};
 uint16_t *w;agc_word address=0;unsigned i;
 if(!s||!io||(unsigned)mission>1u||(unsigned)stage>4u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=(mission==DSKY_NV_LM?lm[stage]:cm[stage])||w[4]!=060000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(060000u|(w[3]>>8))||w[7]||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 for(i=01023;i<=01035u;++i)if(!w[i])return P66_BIAS_OUT_OF_SCOPE;
 if(stage<=DSKY_TWO_INTEGER_SIGN&&w[0777]>14u)return P66_BIAS_OUT_OF_SCOPE;
 if(stage==DSKY_TWO_INTEGER_FIRST&&((w[0154]&040000u&&w[0154]!=077777u)||w[0777]!=(agc_word)(14u-5u*w[0117])))return P66_BIAS_OUT_OF_SCOPE;
 if(stage==DSKY_TWO_INTEGER_SECOND){
  if(w[0140]==2u)address=(agc_word)(w[0150u+w[0117]]&03777u);
  else if(w[0140]==1u&&w[0145]<=01777u)address=w[0145]<01400u?w[0145]:(agc_word)(w[3]+(w[0145]&0377u));
  else return P66_BIAS_OUT_OF_SCOPE;
  if(address<010u||(agc_word)(address-017u)<5u||address>03776u||(address&0377u)==0377u||(w[address+1u]&040000u&&w[address+1u]!=077777u))return P66_BIAS_OUT_OF_SCOPE;
  /* Source saves READLO caller in TEM4 before reading; low TEM4 is then
   * a return address, outside this positive two-digit input domain. */
  if(address+1u==0144u)return P66_BIAS_OUT_OF_SCOPE;
 }
 switch(stage){
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==0
 case DSKY_TWO_INTEGER_BLANK:{
  agc_word caller;enum dsky_five_blank_route blank;cm_native_io c={0};c.superbank=io->superbank;c.interrupt_enabled=io->interrupt_enabled;
  caller=mission==DSKY_NV_LM?02733u:02637u;w[2]=caller;w[5]=mission==DSKY_NV_LM?02536u:02437u;
  if(mission==DSKY_NV_LM)(void)dsky_five_blank(s,io,&blank);else{c.superbank=io->superbank;(void)cm_five_blank(s,&c,&blank);io->interrupt_enabled=c.interrupt_enabled;}
  break;}
#endif
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==1
 case DSKY_TWO_INTEGER_SIGN:{
  enum dsky_sign_output_route sign;w[2]=mission==DSKY_NV_LM?02734u:02640u;w[5]=mission==DSKY_NV_LM?02413u:02314u;(void)dsky_sign_output(s,io,mission,0,&sign);break;}
#endif
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==2
 case DSKY_TWO_INTEGER_FIRST:{
  w[0]=agc_extend(w[0154]);w[2]=mission==DSKY_NV_LM?02736u:02642u;w[5]=mission==DSKY_NV_LM?03306u:03213u;
  data_digits(s,io,mission);break;}
#endif
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==3
 case DSKY_TWO_INTEGER_SECOND:{
  w[0777]=(agc_word)(11u-5u*w[0117]);w[0]=w[0777];w[2]=mission==DSKY_NV_LM?02743u:02647u;w[5]=mission==DSKY_NV_LM?02747u:02655u;
  (void)dsky_read_pair(s,io,mission);w[0]=agc_extend(w[0155]);w[2]=mission==DSKY_NV_LM?02745u:02651u;w[5]=mission==DSKY_NV_LM?03306u:03213u;
  data_digits(s,io,mission);break;}
#endif
#if !defined(DSKY_TWO_INTEGER_ONLY_STAGE) || DSKY_TWO_INTEGER_ONLY_STAGE==4
 case DSKY_TWO_INTEGER_RETURN:{
  w[0]=agc_extend(agc_correct(w[0]));w[1]=agc_extend(agc_correct(w[1]));w[2]=0570;w[4]=062000;w[6]=(agc_word)(062000u|(w[3]>>8));w[5]=02570;break;}
#endif
 default:return P66_BIAS_OUT_OF_SCOPE;
 }
 return P66_BIAS_OK;
}
