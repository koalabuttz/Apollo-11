/* SPDX-License-Identifier: MIT */
#include "dsky_alarm_cycle.h"
static enum p66_bias_status cycle(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_alarm_cycle_route *route,uint8_t cm){uint16_t *w=s->words,pc=w[5],start=cm ? 04161u : 04145u,after=(uint16_t)(start+5u),sw=cm ? 04675u : 04631u,i;
 if(route==0||(pc!=start&&pc!=after&&pc!=sw)||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||(io->control.superbank&0100u)==0||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc==sw&&(w[4]!=062000u||w[0133]!=after||w[0134]>076000u||(w[0134]&01777u)))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));io->wrote_channel11=0;
 if(pc==start){io->channel11=(uint16_t)(io->channel11|0100u);io->wrote_channel11=1;w[01013]=agc_negate(w[01041]);w[0]=agc_extend(w[01013]);w[0133]=after;w[0134]=w[4];w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));w[2]=sw;w[5]=02337;*route=DSKY_ALARM_CYCLE_UPDATE;return P66_BIAS_OK;}
 if(pc==sw){w[0134]=062000;w[0]=agc_extend(agc_correct(w[0]));}
 w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));w[2]=2;w[5]=02002;*route=DSKY_ALARM_CYCLE_ENTER;return P66_BIAS_OK;
}
enum p66_bias_status dsky_alarm_cycle(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_alarm_cycle_route *route){return cycle(s,io,route,0);}
enum exec_status cm_alarm_cycle(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_alarm_cycle_route *route){dsky_charin_io view;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;view.control.superbank=io->superbank;view.control.interrupt_enabled=io->interrupt_enabled;view.channel11=io->channel11;view.wrote_channel11=0;status=cycle(s,&view,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=view.control.superbank;io->interrupt_enabled=view.control.interrupt_enabled;io->channel11=view.channel11;*wrote=view.wrote_channel11;return EXEC_OK;}
