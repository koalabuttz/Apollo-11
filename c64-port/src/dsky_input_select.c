/* SPDX-License-Identifier: MIT */
#include "dsky_input_select.h"
static int dsky_select_entry(uint16_t pc,uint8_t cm){static const uint16_t lm[]={03026,03034,03047,03071,03064,03046,03025,04322,04331,04340},c[]={03026,03034,03047,03071,03064,03046,03025,04336,04345,04354};uint8_t i;for(i=0;i<10u;++i)if(pc==(cm ? c[i] : lm[i]))return i;return -1;}
static int dsky_select_domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t caller;int e;if(!s||!io)return 0;w=s->words;e=dsky_select_entry(w[5],cm);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->restart_light>1u)return 0;
 if(e<7&&(w[4]!=062000u||!(io->control.superbank&0100u)))return 0;
 caller=e==3||e==4||e==5 ? w[0114] : w[2];if(caller<0100u||caller>07777u)return 0;
 if(e==2&&(w[0140]!=1u&&w[0140]!=2u))return 0;
 return !(e==1||(e==2&&w[0140]==2u))||w[0117]<=2u;
}
static uint16_t dsky_select_rotate(uint16_t v,uint8_t left){return left ? (uint16_t)(((uint32_t)v<<1|(v>>14))&077777u) : (uint16_t)((v>>1)|((v&1u)<<14));}
static void dsky_select_cyclic(uint16_t *w,uint8_t left){uint16_t edit=left ? 022u : 020u;uint8_t i,count=(uint8_t)(agc_overflow(w[0]) ? 3u : 4u);w[edit]=dsky_select_rotate(agc_correct(w[0]),left);for(i=0;i<count;++i){w[0]=agc_extend(agc_negate(w[edit]));w[edit]=dsky_select_rotate(w[edit],left);}w[0]=agc_extend(w[edit]);w[edit]=agc_negate(agc_correct(w[0]));}
static void dsky_select_body(apollo_overlay_state *s,uint8_t cm,enum dsky_input_select_route *route){uint16_t *w=s->words,caller=w[2],field,k=0;uint8_t i;int e=dsky_select_entry(w[5],cm);w[1]=agc_extend(agc_correct(w[1]));if(e<3){w[0114]=caller;if(e==1||(e==2&&w[0140]==2u)){k=w[0117];w[1]=k==0 ? 03025u : k==1 ? (cm ? 04336u : 04322u) : (cm ? 04345u : 04331u);}else if(e==0)k=1;field=w[e==1 ? 0153u : 0147u];w[0]=agc_extend((uint16_t)(field&(k==0 ? 037u : k==1 ? 01740u : 076000u)));if(k)dsky_select_cyclic(w,(uint8_t)(k==2));if(e==2)w[0]=agc_add_register(w[0],w[0]);w[5]=caller;w[2]=0115;}
 else if(e<6){caller=w[0114];if(e==3)w[0]=agc_extend((uint16_t)(w[0147]&037u));if(e!=5)w[0]=agc_add_register(w[0],w[0]);w[5]=caller;w[2]=0115;}
 else{if(e==7||e==8)dsky_select_cyclic(w,(uint8_t)(e==8));else if(e==9)for(i=0;i<5u;++i)w[0]=agc_add_register(w[0],w[0]);w[5]=caller;w[2]=3;}*route=DSKY_INPUT_SELECT_CALLER;}
enum p66_bias_status dsky_input_select(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_select_route *route){if(!route||!dsky_select_domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;dsky_select_body(s,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_select(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_select_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!dsky_select_domain(s,&p,1))return EXEC_OUT_OF_SCOPE;dsky_select_body(s,1,route);return EXEC_OK;}
