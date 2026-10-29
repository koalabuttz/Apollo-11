/* SPDX-License-Identifier: MIT */
#include "dsky_source_alarm.h"
static enum p66_bias_status alarm_body(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_alarm_route *route,uint8_t cm){uint16_t *w=s->words,pc=w[5],start=cm ? 03325u : 03420u,paste=cm ? 04140u : 04124u,end=cm ? 05217u : 05155u,caller=w[2];uint8_t lamp=(uint8_t)(pc==(cm ? 04400u : 04364u)||pc==(cm ? 04404u : 04370u)||pc==(cm ? 04410u : 04374u)),extended=(uint8_t)(pc==02120u),god=(uint8_t)(pc==02351u),pred=(uint8_t)(pc==start-2u),direct=(uint8_t)(pc==start+12u);
 if(route==0||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(lamp){if(caller<0100u||caller>07777u)return P66_BIAS_OUT_OF_SCOPE;}
 else if(!(extended||god||pred||direct||pc==start)||w[4]!=(extended ? 066000u : god ? 062000u : 060000u)||(io->control.superbank&0100u)==0)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(god){w[4]=060000;w[6]=(uint16_t)(060000u|(w[3]>>8));}
 if(pred)w[0777]=077754;
 if(!lamp&&!extended&&!direct&&w[0136]==(cm ? 04216u : 04202u)){w[0]=0177777;w[2]=(uint16_t)(start+15u);w[5]=cm ? 05721u : 05652u;*route=DSKY_ALARM_ABORT;return P66_BIAS_OK;}
 if(lamp&&pc==(cm ? 04404u : 04370u))io->channel11=(uint16_t)(io->channel11&077677u);else io->channel11=(uint16_t)(io->channel11|(lamp&&pc==(cm ? 04410u : 04374u) ? 020u : 0100u));
 io->wrote_channel11=1;w[0]=agc_extend(io->channel11);
 if(lamp){w[2]=3;w[5]=caller;*route=DSKY_ALARM_CALLER;}
 else if(extended){w[4]=020000;w[6]=(uint16_t)(020000u|(w[3]>>8));w[2]=cm ? 01272u : 01050u;w[5]=cm ? 03272u : 03050u;*route=DSKY_ALARM_PINBRANCH;}
 else if(!direct&&w[0136]==paste){w[01021]=040000;w[2]=(uint16_t)(start+12u);w[5]=paste;*route=DSKY_ALARM_PASTE;}
 else{w[2]=(uint16_t)(start+14u);w[5]=end;*route=DSKY_ALARM_END;}
 return P66_BIAS_OK;
}
enum p66_bias_status dsky_source_alarm(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_alarm_route *route){return alarm_body(s,io,route,0);}
enum exec_status cm_source_alarm(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_alarm_route *route){dsky_charin_io p;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=alarm_body(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;}
