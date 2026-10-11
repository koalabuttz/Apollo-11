/* SPDX-License-Identifier: MIT */
#include "dsky_input_factor.h"
#include "noun_reader.h"
static int dsky_factor_entry(uint16_t pc){return pc==03174u ? 0 : pc==03206u ? 1 : pc==03207u ? 2 : -1;}
static int dsky_factor_domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t k,index,code;int e;if(!s||!io)return 0;w=s->words;e=dsky_factor_entry(w[5]);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||io->control.superbank>0160u||(io->control.superbank&017u)||!(io->control.superbank&0100u)||io->control.interrupt_enabled>1u||io->restart_light>1u)return 0;if(e==2)return w[0]<=13u;
 if(e==0&&(w[0140]!=1u&&w[0140]!=2u))return 0;
 if(e==0&&w[0140]==2u){k=w[0117];if(k>2u)return 0;index=(uint16_t)((w[0147]>>(k*5u))&037u);code=(uint16_t)((w[0153]>>(k*5u))&037u);}else{index=(uint16_t)(w[0147]&037u);code=(uint16_t)((w[0147]>>5)&037u);}
 return code<=13u&&(e!=0||index<=(cm ? 19u : 28u));
}
static void dsky_factor_select(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm){enum dsky_input_select_route route;if(cm){cm_native_io c={0};c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;(void)cm_input_select(s,&c,&io->wrote_channels,&io->restart_light,&route);}else (void)dsky_input_select(s,io,&route);}
static void dsky_factor_body(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm,enum dsky_input_factor_route *route){static const uint16_t lm[]={062350,061106,061010,061076,061114,061141,061420,061114,065452,061420,061146,061111,061420,061010},c[]={062350,060776,060716,060766,061021,061046,061004,061021,065365,061325,061053,061001,061325,061032};uint16_t *w=s->words,code,bank,cadr;int e=dsky_factor_entry(w[5]);apollo_noun_reader_io n;w[1]=agc_extend(agc_correct(w[1]));
 if(e==0){w[2]=03175;w[5]=03047;dsky_factor_select(s,io,cm);w[0123]=agc_correct(w[0]);bank=w[6];w[0]=03201;w[1]=agc_extend(bank);w[3]=0400;w[4]=064000;w[6]=064001;w[5]=cm ? 02126u : 02147u;n.superbank=io->control.superbank;n.interrupt_enabled=io->control.interrupt_enabled;(void)apollo_noun_reader(s,&n,cm ? APOLLO_FIXED_CM : APOLLO_FIXED_LM,APOLLO_GTSFIN);
  if(w[0140]==2u){w[2]=03205;w[5]=03034;dsky_factor_select(s,io,cm);e=2;}else e=1;
 }
 if(e==1){w[2]=03207;w[5]=03026;dsky_factor_select(s,io,cm);}
 code=agc_correct(w[0]);cadr=cm ? c[code] : lm[code];w[4]=(uint16_t)(cadr&076000u);w[6]=(uint16_t)(w[4]|(w[3]>>8));w[0]=03212;w[2]=(uint16_t)(cadr&01777u);w[5]=(uint16_t)(02000u+w[2]);*route=(enum dsky_input_factor_route)code;
}
enum p66_bias_status dsky_input_factor(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_factor_route *route){if(!route||!dsky_factor_domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;dsky_factor_body(s,io,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_factor(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_factor_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!dsky_factor_domain(s,&p,1))return EXEC_OUT_OF_SCOPE;dsky_factor_body(s,&p,1,route);return EXEC_OK;}
