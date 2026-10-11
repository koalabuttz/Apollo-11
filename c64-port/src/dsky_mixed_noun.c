/* SPDX-License-Identifier: MIT */
#include "dsky_mixed_noun.h"
static uint16_t read_source(apollo_overlay_state *s,uint16_t address){uint16_t value=address<=2u ? agc_correct(s->words[address]) : s->words[address];
 if(address==020u)s->words[address]=(uint16_t)((value>>1)|((value&1u)<<14));
 else if(address==021u)s->words[address]=(uint16_t)((value>>1)|(value&040000u));
 else if(address==022u)s->words[address]=(uint16_t)((((unsigned)value<<1)&077777u)|(value>>14));
 else if(address==023u)s->words[address]=(uint16_t)((value>>7)&0177u);
 return value;
}
static enum p66_bias_status mixed(apollo_overlay_state *s,p66_bias_io *io,enum dsky_mixed_route *route,uint8_t cm){uint16_t *w=s->words,verb=w[01001],nn=w[0146],i,routine,address,value;
 if(route==0||w[5]!=02221u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u||(verb!=077777u&&verb>99u))return P66_BIAS_OUT_OF_SCOPE;
 if(nn!=0&&(verb==077777u||verb<=6u))for(i=0;i<3;++i)if((((unsigned)w[0153]>>(5u*i))&037u)>13u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0]=agc_diminished_abs(nn);
 if(nn==0){w[2]=02224;w[5]=02351;*route=DSKY_MIXED_ALARM;return P66_BIAS_OK;}
 w[0]=agc_add_register(agc_extend(agc_negate(6)),agc_extend(verb));
 if(verb!=077777u&&verb>6u){w[2]=02233;w[5]=02133;*route=DSKY_MIXED_VERB;return P66_BIAS_OK;}
 for(i=3;i!=0;){--i;w[0117]=i;w[0145]=(uint16_t)(0125u+i);w[0122]=w[0150+i];routine=(uint16_t)(((unsigned)w[0153]>>(5u*i))&037u);w[0114]=02243;
  if(i==2u){w[022]=agc_negate(routine);w[1]=cm ? 04345u : 04331u;}else if(i==1u){w[020]=agc_negate(routine);w[1]=cm ? 04336u : 04322u;}else w[1]=03025;
  if(routine==4u||routine==5u||routine==7u||routine==10u||(cm&&routine==13u))w[0122]=agc_correct(agc_add_register(agc_extend(w[0122]),1));
  address=(uint16_t)(w[0122]&03777u);w[3]=(uint16_t)(address&03400u);w[6]=(uint16_t)(062000u|(w[3]>>8));w[0]=(uint16_t)(01400u+(address&0377u));w[2]=3;w[5]=02253;value=read_source(s,address);w[0]=agc_extend(w[0125+i]);w[0125+i]=value;
 }
 w[0]=0;w[2]=02260;w[5]=02133;*route=DSKY_MIXED_VERB;return P66_BIAS_OK;
}
enum p66_bias_status dsky_mixed_noun(apollo_overlay_state *s,p66_bias_io *io,enum dsky_mixed_route *route){return mixed(s,io,route,0);}
enum exec_status cm_mixed_noun(apollo_overlay_state *s,cm_native_io *io,enum dsky_mixed_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=mixed(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
