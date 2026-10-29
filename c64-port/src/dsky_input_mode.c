/* SPDX-License-Identifier: MIT */
#include "dsky_input_mode.h"
static uint16_t right(uint16_t w){return(uint16_t)((w>>1)|(w&040000u));}
static int entry(uint16_t pc,uint8_t cm){return pc==03011u ? 0 : pc==(cm ? 03506u : 03573u) ? 1 : pc==(cm ? 03516u : 03603u) ? 2 : -1;}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;int e;if(!s||!io)return 0;w=s->words;e=entry(w[5],cm);return e>=0&&w[7]==0u&&w[3]<=03400u&&(w[3]&0377u)==0u&&w[4]==(e==0 ? 062000u : 064000u)&&w[6]==(uint16_t)(w[4]|(w[3]>>8))&&w[2]>=0100u&&w[2]<=07777u&&io->control.superbank>=0100u&&io->control.superbank<=0160u&&(io->control.superbank&017u)==0u&&io->control.interrupt_enabled<=1u&&io->restart_light<=1u;}
static void caller(uint16_t *w,enum dsky_input_mode_route *route){w[5]=w[2];w[2]=3;*route=DSKY_INPUT_MODE_CALLER;}
static void force(uint16_t *w,uint8_t cm,enum dsky_input_mode_route *route){w[0]=agc_extend(077746u);w[01041]=077746u;w[2]=cm ? 03521u : 03606u;w[5]=cm ? 04161u : 04145u;*route=DSKY_INPUT_MODE_RECYCLE;}
static void body(apollo_overlay_state *s,uint8_t cm,enum dsky_input_mode_route *route){uint16_t *w=s->words,value;int e=entry(w[5],cm),overflow;w[1]=agc_extend(agc_correct(w[1]));
 if(e==2){force(w,cm,route);return;}
 if(e==1){value=(uint16_t)(w[01000]&034u);w[0]=agc_add_register(value,agc_extend(077743u));value=agc_correct(w[0]);w[0]=agc_extend(agc_diminished_abs(value));if(value==077777u)caller(w,route);else force(w,cm,route);return;}
 w[0117]=agc_correct(w[0]);overflow=agc_overflow(w[0]);w[0]=overflow>0 ? 1u : overflow<0 ? 0177776u : agc_extend(agc_negate(w[01000]));w[021]=right(agc_correct(w[0]));w[0]=agc_extend(agc_negate(w[021]));w[021]=right(w[021]);w[0]=agc_extend(agc_negate(w[021]));w[021]=right(w[021]);value=agc_correct(w[0]);w[0]=agc_extend(agc_diminished_abs(value));
 if(value==0u||value==077777u){caller(w,route);return;}
 w[0]=agc_add_register(w[0],agc_extend(w[0117]));if(w[0]==0u||w[0]==0177777u){caller(w,route);return;}
 w[2]=03025;w[5]=cm ? 04161u : 04145u;*route=DSKY_INPUT_MODE_RECYCLE;
}
enum p66_bias_status dsky_input_mode(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_mode_route *route){if(!route||!domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;body(s,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_mode(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_mode_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!domain(s,&p,1))return EXEC_OUT_OF_SCOPE;body(s,1,route);return EXEC_OK;}
