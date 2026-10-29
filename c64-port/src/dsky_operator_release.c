/* SPDX-License-Identifier: MIT */
#include "dsky_operator_release.h"
static int unsuspend(uint16_t lock,uint16_t monitor){if(lock==077777u)return 1;if(lock>=040000u)return agc_diminished_abs(lock)!=0u;return lock!=0u&&(monitor&020000u)!=0u;}
static int wake_domain(const uint16_t *w,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u,found=0;uint16_t list=w[01043];if(!list)return 1;if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;if(!found&&p>=040000u&&p<077777u&&(loc==list||((loc==0||loc==077777u)&&(list==0||list==077777u)))){if(w[0165u+12u*i]&077610u)return 0;found=1;}}return 1;}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,const cm_native_io *cmio,uint8_t cm){const uint16_t *w;uint8_t suspended;
 if(!s||!io)return 0;
 w=s->words;suspended=(uint8_t)(w[5]==(cm ? 03404u : 03477u)||unsuspend(w[0115],w[01021]));
 if((w[5]!=(cm ? 03366u : 03461u)&&w[5]!=(cm ? 03404u : 03477u))||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->restart_light>1u)return 0;
 return suspended||(!suspended&&wake_domain(w,cm)&&(!cm||w[01043]==0u||cmio->executive_alarm==0u));
}
static void output(dsky_lamp_io *io,uint16_t value){io->channel11=value;io->wrote_channels=(uint16_t)(io->wrote_channels|1u);if(value&01000u)io->restart_light=0;}
static enum p66_bias_status body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_operator_release_route *route){uint16_t *w=s->words,normal=cm ? 03376u : 03471u,start=cm ? 03404u : 03477u;uint8_t direct=(uint8_t)(w[5]==start),suspended=(uint8_t)(direct||unsuspend(w[0115],w[01021]));
 w[1]=agc_extend(agc_correct(w[1]));
 if(!direct){output(io,(uint16_t)(io->channel11&077773u));if(cm)cmio->channel11=io->channel11;
  if(!suspended){enum dsky_lamp_release_route r;w[5]=normal;if(cm){if(cm_lamp_release(s,cmio,&io->wrote_channels,&io->restart_light,&r)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;}else if(dsky_lamp_release(s,io,&r)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;*route=(enum dsky_operator_release_route)r;return P66_BIAS_OK;}
  w[2]=normal;
 }
 w[01012]=0;w[0]=agc_extend(agc_diminished_abs(w[01042]));
 if(w[01042]==077777u){w[5]=cm ? 03412u : 03505u;*route=DSKY_OPERATOR_RELEASE_BLANK;return P66_BIAS_OK;}
 if(w[01042]==0u){w[0144]=(uint16_t)(start+5u);if(w[01043]==0u){io->control.interrupt_enabled=1;output(io,(uint16_t)(io->channel11&077757u));}w[0]=0;w[2]=(uint16_t)(start+6u);}
 else w[2]=(uint16_t)(start+((w[01042]&040000u)?6u:4u));
 w[5]=cm ? 05217u : 05155u;*route=DSKY_OPERATOR_RELEASE_END;if(cm){cmio->interrupt_enabled=io->control.interrupt_enabled;cmio->channel11=io->channel11;}return P66_BIAS_OK;
}
enum p66_bias_status dsky_operator_release(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_operator_release_route *route){if(!route||!domain(s,io,0,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0,route);}
enum exec_status cm_operator_release(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_operator_release_route *route){dsky_lamp_io p;enum p66_bias_status status;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;if(!domain(s,&p,io,1))return EXEC_OUT_OF_SCOPE;status=body(s,&p,io,1,route);if(status)return EXEC_OUT_OF_SCOPE;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
