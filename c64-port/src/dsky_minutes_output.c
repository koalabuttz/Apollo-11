/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_minutes_output.h"
#include "dsky_fraction_pair.h"
#include "dsky_pinball.h"
#include "guidance_product.h"
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0
static agc_word magnitude(agc_word value){return (value&AGC_SIGN)?agc_negate(value):value;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0
static agc_register carry(int sign){return sign>0?1u:0177776u;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0
static void agree(uint16_t *w){int sign=agc_to_int(w[0154]),ov;agc_register a;agc_word limit;uint8_t i;if(!sign)sign=agc_to_int(w[0155]);if(!sign){w[0154]=w[0155]=w[0156]=0;return;}limit=sign>0?037777:040000;a=agc_add_register(agc_extend(limit),carry(sign));for(i=2;i>0;--i){a=agc_add_register(a,agc_extend(w[0154u+i]));w[0154u+i]=agc_correct(a);ov=agc_overflow(a);a=agc_add_register(ov?carry(ov):0,agc_extend(limit));}a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);if(!agc_overflow(a))w[0155]=agc_correct(a);}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0 || DSKY_MINUTES_ONLY_STAGE==4
static void dmp(uint16_t *w,agc_word high,agc_word low,agc_word address){p66_preparation p;p66_scale_scratch sc;agc_dp factor={high,low};uint8_t i;memset(&p,0,sizeof p);for(i=0;i<3u;++i)p.mpac[i]=w[0154u+i];sc.mptemp=w[0135];sc.ovfind=w[0121];(void)p66_scalar_product(&p,&sc,&factor);for(i=0;i<3u;++i)w[0154u+i]=p.mpac[i];w[0135]=sc.mptemp;w[0116]=address;w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0 || DSKY_MINUTES_ONLY_STAGE==1
static void accumulate(agc_register_pair *r,uint16_t *at){agc_dp p={at[0],at[1]};agc_double_add(r,&p);at[0]=p.high;at[1]=p.low;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==1
static void shortmp(uint16_t *w,agc_word factor){agc_word minor=w[0155],major=w[0154];agc_register_pair r;w[0135]=factor;r=agc_multiply(agc_extend(factor),w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(minor),factor);accumulate(&r,w+0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(major),factor);accumulate(&r,w+0154);w[0]=r.a;w[1]=r.l;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0
static void left(uint16_t *w){agc_register_pair r={agc_extend(w[0155]),agc_extend(w[0156])};int ov;accumulate(&r,w+0155);w[0]=agc_add_register(r.a,agc_extend(w[0154]));w[0]=agc_add_register(w[0],agc_extend(w[0154]));w[0154]=agc_correct(w[0]);ov=agc_overflow(w[0]);if(ov){w[0]=carry(ov);w[0162]=agc_correct(w[0]);}w[1]=r.l;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==2 || DSKY_MINUTES_ONLY_STAGE==3 || DSKY_MINUTES_ONLY_STAGE==5
static void call(uint16_t *w,enum dsky_nv_mission mission,agc_word returned){w[0]=agc_extend(agc_correct(w[0]));w[1]=agc_extend(agc_correct(w[1]));w[0133]=returned;w[0134]=064000;w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[2]=mission==DSKY_NV_LM?04631u:04675u;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==2 || DSKY_MINUTES_ONLY_STAGE==3 || DSKY_MINUTES_ONLY_STAGE==5
static void returned(uint16_t *w,agc_word returned){w[0134]=060000;w[4]=064000;w[6]=(agc_word)(064000u|(w[3]>>8));w[2]=0134;w[5]=returned;}
#endif
enum p66_bias_status dsky_minutes_output(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_minutes_stage phase){static const agc_word pcs[2][6]={{03303,03343,03347,03351,03361,03371},{03216,03256,03262,03264,03274,03304}};const agc_word *pc;uint16_t *w;unsigned i;if(s==0||io==0||(unsigned)mission>1u||(unsigned)phase>=6u)return P66_BIAS_OUT_OF_SCOPE;
#ifdef DSKY_MINUTES_ONLY_STAGE
 if((unsigned)phase!=DSKY_MINUTES_ONLY_STAGE)return P66_BIAS_OUT_OF_SCOPE;
#endif
 pc=pcs[mission];w=s->words;if(w[5]!=pc[phase]||w[4]!=064000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(064000u|(w[3]>>8))||w[7]||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;for(i=3;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==0
 if(phase==0u){uint16_t address,eb,high,low;agc_register a;agc_register_pair r;uint8_t negative,limited;if((w[0140]!=1u&&w[0140]!=2u)||w[0777]!=(agc_word)(14u-5u*w[0117]))return P66_BIAS_OUT_OF_SCOPE;eb=(uint16_t)(w[3]>>8);if(w[0140]==2u){address=(uint16_t)(w[0150u+w[0117]]&03777u);eb=(uint16_t)(address>>8);}else{if(w[0145]>01777u)return P66_BIAS_OUT_OF_SCOPE;address=w[0145]<01400u?w[0145]:(uint16_t)(eb*0400u+(w[0145]&0377u));}if(address<010u||address>03776u||(address&0377u)==0377u)return P66_BIAS_OUT_OF_SCOPE;w[3]=(agc_word)(eb*0400u);w[6]=(agc_word)(064000u|eb);w[0133]=(agc_word)(pc[0]+2u);w[0134]=060000;w[0154]=w[address];w[0155]=w[address+1u];w[0156]=0;agree(w);high=magnitude(w[0154]);low=magnitude(w[0155]);negative=(uint8_t)((w[0154]&AGC_SIGN)!=0);limited=(uint8_t)(high>025u||(high==025u&&low>=036652u));if(limited){w[0154]=negative?agc_negate(025):025;w[0155]=negative?agc_negate(037016):037016;}else{r.a=0;r.l=(w[0155]&AGC_SIGN)?agc_extend(agc_negate(062)):062;accumulate(&r,w+0154);}w[0144]=pc[1];dmp(w,025660,031742,mission==DSKY_NV_LM?03267u:03202u);w[01007]=w[0154];w[01010]=w[0155];left(w);left(w);a=agc_extend(w[0154]);w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0]=a;w[2]=0145;w[5]=pc[1];return P66_BIAS_OK;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==1
 if(phase==1u){if(w[0777]!=(agc_word)(14u-5u*w[0117]))return P66_BIAS_OUT_OF_SCOPE;shortmp(w,023147);w[0]=agc_add_register(agc_extend(agc_negate(3)),agc_extend(w[0777]));w[0777]=agc_correct(w[0]);w[2]=3;w[5]=pc[2];return P66_BIAS_OK;}
#endif
 for(i=0;i<11u;++i)if(w[01023u+i]==0)return P66_BIAS_OUT_OF_SCOPE;
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==2 || DSKY_MINUTES_ONLY_STAGE==5
 if(phase==2u||phase==5u){enum dsky_decimal_general_route route;if(w[0777]!=(agc_word)(14u-5u*w[0117]-(phase==2u?3u:0u)))return P66_BIAS_OUT_OF_SCOPE;call(w,mission,(agc_word)(pc[phase]+2u));w[5]=mission==DSKY_NV_LM?03266u:03173u;(void)dsky_fraction_pair(s,io,mission,&route);returned(w,(agc_word)(pc[phase]+2u));if(phase==5u){w[0]=agc_extend(agc_correct(w[0]));w[1]=agc_extend(agc_correct(w[1]));w[2]=0570;w[4]=062000;w[6]=(agc_word)(062000u|(w[3]>>8));w[5]=02570;}return P66_BIAS_OK;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==3
 if(phase==3u){p66_bias_io physical;w[0124]=0;w[0]=agc_extend((agc_word)(12u-5u*w[0117]));w[0143]=agc_correct(w[0]);call(w,mission,pc[4]);w[5]=mission==DSKY_NV_LM?03322u:03227u;physical.superbank=0100;physical.interrupt_enabled=io->interrupt_enabled;(void)dsky_pinball_write(s,&physical,DSKY_DSPIN);io->interrupt_enabled=physical.interrupt_enabled;returned(w,pc[4]);return P66_BIAS_OK;}
#endif
#if !defined(DSKY_MINUTES_ONLY_STAGE) || DSKY_MINUTES_ONLY_STAGE==4
 {agc_register_pair r;agc_register a;agc_word high,low;w[0144]=(agc_word)(pc[4]+1u);r=agc_multiply(agc_extend(w[01010]),4);r=agc_multiply(r.a,010000);w[0155]=agc_correct(r.l);w[0154]=w[01007];dmp(w,02104,010422,mission==DSKY_NV_LM?03275u:03210u);a=agc_extend(w[0155]);w[0155]=agc_correct(w[1]);r=agc_multiply(a,023346);high=w[0154];low=w[0155];w[0154]=agc_correct(r.a);w[0155]=agc_correct(r.l);w[0]=agc_extend(high);w[1]=agc_extend(low);w[0777]=(agc_word)(14u-5u*w[0117]);w[0]=w[0777];w[2]=0145;w[5]=pc[5];return P66_BIAS_OK;}
#else
 return P66_BIAS_OUT_OF_SCOPE;
#endif
}
