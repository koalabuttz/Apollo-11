/* SPDX-License-Identifier: MIT */
#include "dsky_major_request.h"
static enum p66_bias_status major(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_major_request_route *route,uint8_t cm){uint16_t *w=s->words,pc=w[5],caller=w[2],i,end=cm ? 05217u : 05155u,nv=cm ? 04216u : 04202u,paste=cm ? 04140u : 04124u;p66_bias_io physical;
 if(route==0||(pc!=03430u&&pc!=03431u&&pc!=03452u)||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||(io->control.superbank&0100u)==0||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc!=03431u){if(pc==03452u&&(caller<0100u||caller>07777u))return P66_BIAS_OUT_OF_SCOPE;if(w[0136]!=end&&w[0136]!=nv&&w[0136]!=paste)return P66_BIAS_OUT_OF_SCOPE;}
 w[1]=agc_extend(agc_correct(w[1]));io->wrote_channel11=0;
 if(pc==03431u){w[0]=agc_add_register(020u,agc_extend(w[0777]));if(w[0]!=0&&w[0]!=0177777u){w[2]=03436;w[5]=cm ? 04161u : 04145u;*route=DSKY_MAJOR_REQUEST_ALMCYCLE;return P66_BIAS_OK;}w[0154]=w[01002];w[01002]=0;w[0133]=03445;}
 else{if(pc==03430u)caller=03431;w[01013]=agc_negate(caller);w[01002]=0;w[0133]=03462;}
 w[0777]=021;w[0134]=062000;w[0]=pc==03431u ? 021u : 0u;w[2]=cm ? 04675u : 04631u;w[4]=060000;w[6]=(uint16_t)(060000u|(w[3]>>8));w[5]=cm ? 02502u : 02601u;physical.superbank=0100;physical.interrupt_enabled=io->control.interrupt_enabled;(void)dsky_pinball_write(s,&physical,DSKY_2BLANK);io->control.interrupt_enabled=physical.interrupt_enabled;w[0134]=060000;w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));
 if(pc==03431u){w[0777]=077754;w[0]=agc_extend(w[0154]);w[2]=cm ? 011u : 040u;w[4]=010000;w[6]=(uint16_t)(010000u|(w[3]>>8));w[5]=cm ? 02011u : 02040u;*route=DSKY_MAJOR_REQUEST_V37;return P66_BIAS_OK;}
 io->channel11=(uint16_t)(io->channel11|040u);io->wrote_channel11=1;w[01000]=1;w[0]=1;w[2]=0137;w[5]=w[0136];*route=w[0136]==end ? DSKY_MAJOR_REQUEST_END : w[0136]==nv ? DSKY_MAJOR_REQUEST_NVSUB_END : DSKY_MAJOR_REQUEST_PASTE;return P66_BIAS_OK;
}
enum p66_bias_status dsky_major_request(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_major_request_route *route){return major(s,io,route,0);}
enum exec_status cm_major_request(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_major_request_route *route){dsky_charin_io view;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;view.control.superbank=io->superbank;view.control.interrupt_enabled=io->interrupt_enabled;view.channel11=io->channel11;view.wrote_channel11=0;status=major(s,&view,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=view.control.superbank;io->interrupt_enabled=view.control.interrupt_enabled;io->channel11=view.channel11;*wrote=view.wrote_channel11;return EXEC_OK;}
