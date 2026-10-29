/* SPDX-License-Identifier: MIT */
#include "dsky_request_control.h"
static enum p66_bias_status request(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_request_control_route *route,uint8_t cm){uint16_t *w=s->words,pc=w[5],i,novac=cm ? 05134u : 05072u,find=cm ? 05147u : 05105u,wait=cm ? 05245u : 05203u,service,priority;
 if(route==0||(pc!=03466u&&pc!=03473u&&pc!=03476u&&pc!=03477u&&pc!=03500u&&pc!=03510u&&pc!=03512u&&pc!=03515u)||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||(io->control.superbank&0100u)==0||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc==03473u&&w[0]!=novac&&w[0]!=find)return P66_BIAS_OUT_OF_SCOPE;
 if((pc==03476u||pc==03515u)&&w[0]!=agc_extend(agc_correct(w[0])))return P66_BIAS_OUT_OF_SCOPE;
 if(pc==03500u&&w[0154]!=novac&&w[0154]!=find&&w[0154]!=wait)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));io->wrote_channel11=0;
 if(pc==03500u){service=w[0154];w[0157]=cm ? 05217u : 05155u;w[1]=agc_extend(w[0156]);w[0155]=w[01046];w[0156]=w[01047];w[0]=agc_extend(w[0160]);w[2]=0155;w[5]=service;io->control.interrupt_enabled=0;*route=service==novac ? DSKY_REQUEST_NOVAC : service==find ? DSKY_REQUEST_FINDVAC : DSKY_REQUEST_WAITLIST;return P66_BIAS_OK;}
 if(pc==03466u||pc==03473u||pc==03510u){service=pc==03473u ? w[0] : pc==03510u||(w[01045]&1u) ? find : novac;w[0154]=service;priority=(uint16_t)(w[01045]&077776u);w[0160]=priority;w[0]=agc_extend(priority);}
 else if(pc==03512u){w[0154]=wait;w[0160]=w[01045];w[0]=agc_extend(w[01045]);}
 else if(pc==03476u||pc==03515u)w[0160]=agc_correct(w[0]);
 w[2]=03500;w[5]=cm ? 04473u : 04457u;*route=DSKY_REQUEST_RELEASE;return P66_BIAS_OK;
}
enum p66_bias_status dsky_request_control(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_request_control_route *route){return request(s,io,route,0);}
enum exec_status cm_request_control(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_request_control_route *route){dsky_charin_io view;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;view.control.superbank=io->superbank;view.control.interrupt_enabled=io->interrupt_enabled;view.channel11=io->channel11;view.wrote_channel11=0;status=request(s,&view,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=view.control.superbank;io->interrupt_enabled=view.control.interrupt_enabled;io->channel11=view.channel11;*wrote=view.wrote_channel11;return EXEC_OK;}
