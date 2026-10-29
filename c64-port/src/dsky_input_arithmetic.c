/* SPDX-License-Identifier: MIT */
#include "dsky_input_arithmetic.h"
#include "agc_multiword.h"
static int entry(uint16_t pc,uint8_t cm){static const uint16_t lm[]={04404,03153,03156,03163,03165,03173,03174},c[]={04420,03060,03063,03070,03072,03100,03101};uint8_t i;for(i=0;i<7u;++i)if(pc==(cm ? c[i] : lm[i]))return i;return -1;}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t caller;int e;if(!s||!io)return 0;w=s->words;e=entry(w[5],cm);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->restart_light>1u)return 0;
 if(e!=0&&(w[4]!=060000u||!(io->control.superbank&0100u)))return 0;
 caller=e==2 ? w[0124] : w[2];if(caller<0100u||caller>07777u)return 0;
 return (e!=1&&e!=2)||w[0]<=14u;
}
static void dsky_input_shift(uint16_t *w){agc_register_pair r={agc_extend(w[0155]),agc_extend(w[0156])};agc_dp d={w[0155],w[0156]};int overflow;agc_double_add(&r,&d);w[0155]=d.high;w[0156]=d.low;w[1]=r.l;w[0]=agc_add_register(r.a,agc_extend(w[0154]));w[0]=agc_add_register(w[0],agc_extend(w[0154]));w[0154]=agc_correct(w[0]);overflow=agc_overflow(w[0]);if(overflow){w[0]=overflow>0 ? 1u : 0177776u;w[0162]=agc_correct(w[0]);}}
static void round_pair(uint16_t *w,uint8_t partial){uint16_t value;int overflow;if(!partial){value=w[0155];w[0155]=agc_correct(w[0]);w[0]=agc_extend(value);w[0]=agc_add_register(w[0],w[0]);}
 w[0155]=agc_correct(w[0]);overflow=agc_overflow(w[0]);if(!overflow)return;w[0]=agc_add_register(overflow>0 ? 1u : 0177776u,agc_extend(w[0154]));w[0154]=agc_correct(w[0]);overflow=agc_overflow(w[0]);if(overflow){w[0]=overflow>0 ? 1u : 0177776u;w[0162]=agc_correct(w[0]);}}
static void body(apollo_overlay_state *s,uint8_t cm,enum dsky_input_arithmetic_route *route){uint16_t *w=s->words,caller=w[2],flag,start=cm ? 03101u : 03174u;uint8_t n;int e=entry(w[5],cm);w[1]=agc_extend(agc_correct(w[1]));
 if(e==0)dsky_input_shift(w);else if(e==1||e==2){n=(uint8_t)w[0];if(e==1)w[0124]=caller;else caller=w[0124];do{w[0123]=n;dsky_input_shift(w);w[0]=agc_extend(agc_diminished_abs(n));}while(n--!=0u);w[2]=0125;w[5]=caller;*route=DSKY_INPUT_ARITHMETIC_CALLER;return;}
 else if(e==3||e==4)round_pair(w,(uint8_t)(e==4));else if(e==6){flag=w[0162];w[0]=agc_extend(agc_diminished_abs(flag));if(flag==077777u){w[5]=(uint16_t)(start+4u);*route=DSKY_INPUT_ARITHMETIC_FALLTHROUGH;return;}if(flag){w[2]=(uint16_t)(start+((flag&040000u) ? 4u : 2u));w[5]=cm ? 04161u : 04145u;*route=DSKY_INPUT_ARITHMETIC_RECYCLE;return;}}
 w[5]=caller;w[2]=3;*route=DSKY_INPUT_ARITHMETIC_CALLER;
}
enum p66_bias_status dsky_input_arithmetic(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_input_arithmetic_route *route){if(!route||!domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;body(s,0,route);return P66_BIAS_OK;}
enum exec_status cm_input_arithmetic(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_input_arithmetic_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!domain(s,&p,1))return EXEC_OUT_OF_SCOPE;body(s,1,route);return EXEC_OK;}
