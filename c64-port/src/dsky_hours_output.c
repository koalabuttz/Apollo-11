/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "dsky_hours_output.h"
#include "dsky_read_pair.h"
#include "guidance_product.h"
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0
static agc_register carry(int sign){return sign>0?1u:0177776u;}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0
static void agree(uint16_t *w)
{int sign=agc_to_int(w[0154]),ov;agc_register a;agc_word limit;unsigned i;if(!sign)sign=agc_to_int(w[0155]);if(!sign){w[0154]=w[0155]=w[0156]=0;return;}limit=sign>0?037777u:040000u;a=agc_add_register(agc_extend(limit),carry(sign));for(i=2;i>0;--i){a=agc_add_register(a,agc_extend(w[0154u+i]));w[0154u+i]=agc_correct(a);ov=agc_overflow(a);a=agc_add_register(ov?carry(ov):0,agc_extend(limit));}a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);if(!agc_overflow(a))w[0155]=agc_correct(a);}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0 || DSKY_HOURS_ONLY_STAGE==1 || DSKY_HOURS_ONLY_STAGE==3
static void product(uint16_t *w,agc_word hi,agc_word lo,agc_word address)
{p66_preparation p;p66_scale_scratch sc;agc_dp factor={hi,lo};unsigned i;memset(&p,0,sizeof p);for(i=0;i<3u;++i)p.mpac[i]=w[0154u+i];sc.mptemp=w[0135];sc.ovfind=w[0121];(void)p66_scalar_product(&p,&sc,&factor);for(i=0;i<3u;++i)w[0154u+i]=p.mpac[i];w[0135]=sc.mptemp;w[0116]=address;w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0 || DSKY_HOURS_ONLY_STAGE==3 || DSKY_HOURS_ONLY_STAGE==5
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t at)
{agc_dp p={w[at],w[at+1u]};agc_double_add(r,&p);w[at]=p.high;w[at+1u]=p.low;}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0
static void left(uint16_t *w)
{agc_register_pair r={agc_extend(w[0155]),agc_extend(w[0156])};int ov;accumulate(&r,w,0155);w[0]=agc_add_register(r.a,agc_extend(w[0154]));w[0]=agc_add_register(w[0],agc_extend(w[0154]));w[0154]=agc_correct(w[0]);ov=agc_overflow(w[0]);if(ov){w[0]=carry(ov);w[0162]=agc_correct(w[0]);}w[1]=r.l;}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==3 || DSKY_HOURS_ONLY_STAGE==5
static void short_product(uint16_t *w,agc_word factor)
{agc_register_pair r;agc_word hi=w[0154],lo=w[0155];w[0135]=factor;if(factor==077777u){w[0154]=w[0155]=w[0156]=077777;w[0]=0177777;return;}r=agc_multiply(agc_extend(factor),w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(lo),factor);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(hi),factor);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;}
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==2 || DSKY_HOURS_ONLY_STAGE==4 || DSKY_HOURS_ONLY_STAGE==6
static void display(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,agc_word returned)
{uint16_t *w=s->words;enum dsky_decimal_general_route route;w[0]=agc_extend(agc_correct(w[0]));w[1]=agc_extend(agc_correct(w[1]));w[0133]=returned;w[0134]=064000;w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[2]=mission==DSKY_NV_LM?04631u:04675u;w[5]=mission==DSKY_NV_LM?03226u:03133u;(void)dsky_decimal_raw(s,io,mission,1,&route);w[0134]=060000;w[4]=064000;w[6]=(agc_word)(064000u|(w[3]>>8));w[2]=0134;w[5]=returned;}
#endif
enum p66_bias_status dsky_hours_output(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_hours_stage stage)
{
 static const agc_word pc[2][8]={{03230,03234,03240,03242,03253,03255,03264,03266},{03143,03147,03153,03155,03166,03170,03177,03201}};
 uint16_t *w;unsigned i;agc_word address=0;
 if(!s||!io||(unsigned)mission>1u||(unsigned)stage>7u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;if(w[5]!=pc[mission][stage]||w[4]!=064000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(064000u|(w[3]>>8))||w[7]||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 if(stage==DSKY_HOURS_FRESH){if(w[0140]==2u)address=(agc_word)(w[0150u+w[0117]]&03777u);else if(w[0140]==1u&&w[0145]<=01777u)address=w[0145]<01400u?w[0145]:(agc_word)(w[3]+(w[0145]&0377u));else return P66_BIAS_OUT_OF_SCOPE;if(address<010u||(address>=017u&&address<=023u)||address>03776u||(address&0377u)==0377u)return P66_BIAS_OUT_OF_SCOPE;}
 if(stage==DSKY_HOURS_SECONDS_DISPLAY||stage==DSKY_HOURS_MINUTES_DISPLAY||stage==DSKY_HOURS_HOURS_DISPLAY){for(i=01023;i<=01035u;++i)if(!w[i])return P66_BIAS_OUT_OF_SCOPE;if(w[0777]!=(stage==DSKY_HOURS_SECONDS_DISPLAY?4u:stage==DSKY_HOURS_MINUTES_DISPLAY?9u:14u))return P66_BIAS_OUT_OF_SCOPE;}
 switch(stage){
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==0
 case DSKY_HOURS_FRESH:{agc_word a;w[0133]=(agc_word)(pc[mission][0]+2u);w[0134]=060000;w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[2]=mission==DSKY_NV_LM?04631u:04675u;w[5]=mission==DSKY_NV_LM?02747u:02655u;(void)dsky_read_pair(s,io,mission);w[4]=064000;w[6]=(agc_word)(064000u|(w[3]>>8));agree(w);w[0144]=pc[mission][1];product(w,025660,031742,mission==DSKY_NV_LM?03267u:03202u);w[01007]=w[0154];w[01010]=w[0155];left(w);left(w);a=w[0154];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0]=agc_extend(a);w[2]=0145;}
break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==1
 case DSKY_HOURS_SECONDS:{product(w,01727,01217,mission==DSKY_NV_LM?03271u:03204u);w[0777]=4;w[0]=4;w[2]=3;}
break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==2
 case DSKY_HOURS_SECONDS_DISPLAY:{display(s,io,mission,pc[mission][3]);
}break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==3
 case DSKY_HOURS_MINUTES:{agc_word a;agc_register_pair r;w[0144]=(agc_word)(pc[mission][3]+1u);r=agc_multiply(agc_extend(w[01010]),4);r=agc_multiply(r.a,010000);w[0155]=agc_correct(r.l);w[0154]=w[01007];product(w,02104,010422,mission==DSKY_NV_LM?03275u:03210u);w[01007]=w[0154];w[0154]=011;a=w[0155];w[0155]=032445;short_product(w,a);w[0]=9;w[0777]=9;w[2]=3;}
break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==4
 case DSKY_HOURS_MINUTES_DISPLAY:{display(s,io,mission,pc[mission][5]);
}break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==5
 case DSKY_HOURS_HOURS:{agc_word oldhi,oldlo;oldhi=w[0154];oldlo=w[0155];w[0154]=05174;w[0155]=013261;w[0]=agc_extend(oldhi);w[1]=agc_extend(oldlo);short_product(w,w[01007]);w[0]=14;w[0777]=14;w[2]=3;}
break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==6
 case DSKY_HOURS_HOURS_DISPLAY:{display(s,io,mission,pc[mission][7]);
}break;
#endif
#if !defined(DSKY_HOURS_ONLY_STAGE) || DSKY_HOURS_ONLY_STAGE==7
 case DSKY_HOURS_RETURN:{w[2]=(agc_word)(pc[mission][7]+1u);w[5]=0136;return P66_BIAS_OK;}
#endif
 default:return P66_BIAS_OUT_OF_SCOPE;
 }
 w[5]=pc[mission][(unsigned)stage+1u];return P66_BIAS_OK;
}
