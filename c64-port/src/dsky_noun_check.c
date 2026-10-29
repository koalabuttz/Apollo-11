/* SPDX-License-Identifier: MIT */
#include "dsky_noun_check.h"
static int entry(uint16_t pc){static const uint16_t entries[]={02424,02426,02437,02443,02452,02460,02515};uint8_t i;for(i=0;i<7u;++i)if(pc==entries[i])return i;return -1;}
static int component(uint16_t value){return value==0u||value>=077775u;}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io){const uint16_t *w;int e;uint16_t argument,caller;if(!s||!io)return 0;w=s->words;e=entry(w[5]);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->restart_light>1u||(w[0140]!=1u&&w[0140]!=2u))return 0;caller=e==1 ? w[1] : w[2];if(caller<0100u||caller>07777u)return 0;
 if(e==0||e==2){argument=agc_correct(w[0]);if(w[0]!=agc_extend(argument)||!component(argument))return 0;}else if(e==1&&!component(w[0123]))return 0;return 1;}
static void transfer(uint16_t *w,uint16_t pc,uint16_t q,enum dsky_noun_check_route r,enum dsky_noun_check_route *route){w[5]=pc;w[2]=q;*route=r;}
static void body(apollo_overlay_state *s,uint8_t cm,enum dsky_noun_check_route *route){uint16_t *w=s->words,code=(uint16_t)((w[w[0140]==1u ? 0147u : 0146u]>>10)&037u),mask=(uint16_t)(code<<10),caller=w[2],value;int e=entry(w[5]);w[1]=agc_extend(agc_correct(w[1]));
 if(e==6){w[0]=agc_extend(mask);transfer(w,caller,3,DSKY_NOUN_CHECK_CALLER,route);return;}
 if(e==0||e==2){w[0123]=agc_correct(w[0]);w[1]=caller;}else if(e==1)caller=w[1];else if(e==4||e==5)w[1]=caller;
 if(e==3)w[0156]=caller;else if(e==2)w[0156]=02442;
 if(e==2||e==3||e==4){w[0]=(code&010u) ? 017777u : 0u;if(code&010u){transfer(w,e==4 ? (cm ? 04161u : 04145u) : 02351u,e==4 ? 02457u : 02451u,e==4 ? DSKY_NOUN_CHECK_RECYCLE : DSKY_NOUN_CHECK_ALARM,route);return;}
  if(e==3){transfer(w,caller,0157,DSKY_NOUN_CHECK_CALLER,route);return;}if(e==4){transfer(w,caller,2,DSKY_NOUN_CHECK_CALLER,route);return;}
 }
 if(e==5){w[0]=agc_extend(agc_diminished_abs(mask));if(code&020u)transfer(w,02351,02466,DSKY_NOUN_CHECK_ALARM,route);else transfer(w,caller,2,DSKY_NOUN_CHECK_CALLER,route);return;}
 /* Exact LEFT5 CYL read/write debris after the masked type field. */
 w[022]=agc_negate(code);w[0]=agc_add_register((uint16_t)(code&3u),agc_extend(w[0123]));value=agc_correct(w[0]);w[0]=agc_extend(agc_diminished_abs(value));
 if(value==0u)transfer(w,cm ? 05747u : 05705u,02435,DSKY_NOUN_CHECK_HOLE,route);else if(value!=077777u&&(value&040000u))transfer(w,02351,02436,DSKY_NOUN_CHECK_ALARM,route);else transfer(w,caller,2,DSKY_NOUN_CHECK_CALLER,route);
}
enum p66_bias_status dsky_noun_check(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_noun_check_route *route){if(!route||!domain(s,io))return P66_BIAS_OUT_OF_SCOPE;body(s,0,route);return P66_BIAS_OK;}
enum exec_status cm_noun_check(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_noun_check_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.restart_light=*restart;if(!domain(s,&p))return EXEC_OUT_OF_SCOPE;body(s,1,route);return EXEC_OK;}
