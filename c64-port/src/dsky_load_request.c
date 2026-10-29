/* SPDX-License-Identifier: MIT */
#include "dsky_load_request.h"
static int entry(uint16_t pc){static const uint16_t entries[]={02616,02703,02732,02743,02760,02624,02627,02711};uint8_t i;for(i=0;i<8u;++i)if(pc==entries[i])return i;return -1;}
static uint16_t component(int e){return(uint16_t)(e==3||e==5||e==7 ? 1u : e==4||e==6 ? 2u : 0u);}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){const uint16_t *w;uint16_t i,code,k;int e;if(!s||!io)return 0;w=s->words;e=entry(w[5]);if(e<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->restart_light>1u)return 0;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return 0;
 if(e==0||e==1||e==3||e==4){if(w[0140]!=1u&&w[0140]!=2u)return 0;code=(uint16_t)(w[w[0140]==1u ? 0147u : 0146u]>>10);if((code&3u)<(uint16_t)(e==0||e==4 ? 2u : 1u)||((e==0||e==1)&&(code&020u)))return 1;}
 if((e==0||e==1||e>=5)&&w[01034]==0u)return 1;
 k=component(e);if(w[01023u+(k==0u ? 7u : 2u)]==0u)return 1;
 return w[0136]==(cm ? 05217u : 05155u)||w[0136]==(cm ? 04216u : 04202u)||w[0136]==(cm ? 04140u : 04124u);
}
static enum p66_bias_status noun(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_noun_check_route *route){return cm ? (cm_noun_check(s,cmio,&io->wrote_channels,&io->restart_light,route)==EXEC_OK ? P66_BIAS_OK : P66_BIAS_OUT_OF_SCOPE) : dsky_noun_check(s,io,route);}
static enum p66_bias_status body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_load_request_route *route){static const uint16_t check_return[]={02620,02705,0,02745,02762},update_return[]={02623,02710,0,0,0,02626,02631,02713},request_return[]={02624,02711,02733,02750,02765,02627,02632,02714};uint16_t *w=s->words,k;int e=entry(w[5]);enum dsky_noun_check_route check;enum dsky_nv_update_route update;enum dsky_request_route request;dsky_charin_io p;
 w[1]=agc_extend(agc_correct(w[1]));
 if(e==0||e==1||e==3||e==4){w[0]=agc_extend((e==0||e==4) ? 077775u : 077776u);w[2]=check_return[e];w[5]=02424;if(noun(s,io,cmio,cm,&check)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;if(check!=DSKY_NOUN_CHECK_CALLER){*route=check==DSKY_NOUN_CHECK_HOLE ? DSKY_LOAD_REQUEST_HOLE : DSKY_LOAD_REQUEST_ALARM;return P66_BIAS_OK;}
  if(e<2){w[2]=(uint16_t)(check_return[e]+1u);w[5]=02460;if(noun(s,io,cmio,cm,&check)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;if(check!=DSKY_NOUN_CHECK_CALLER){*route=DSKY_LOAD_REQUEST_ALARM;return P66_BIAS_OK;}}
  else{w[0]=0140000;w[01015]=040000;}
 }
 k=component(e);
 if(update_return[e]){w[0]=(uint16_t)(21u+k);w[2]=update_return[e];w[5]=02337;if(dsky_nv_update_general(s,&io->control,cm ? DSKY_NV_CM : DSKY_NV_LM,DSKY_NV_VERB_STORE,&update)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;if(cm){cmio->superbank=io->control.superbank;cmio->interrupt_enabled=io->control.interrupt_enabled;}if(update==DSKY_NV_UPDATE_CCSHOLE){*route=DSKY_LOAD_REQUEST_HOLE;return P66_BIAS_OK;}}
 w[2]=request_return[e];w[5]=(uint16_t)(02303u+2u*k);
 if(cm){uint16_t event=0;if(cm_address_request(s,cmio,&event,&request)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;io->wrote_channels=(uint16_t)(io->wrote_channels|event);}else{p.control=io->control;p.channel11=io->channel11;p.wrote_channel11=0;if(dsky_address_request(s,&p,&request)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;io->control=p.control;io->channel11=p.channel11;io->wrote_channels=(uint16_t)(io->wrote_channels|p.wrote_channel11);}
 if(request!=DSKY_REQUEST_CCS_HOLE&&(io->channel11&01000u))io->restart_light=0;
 *route=request==DSKY_REQUEST_CCS_HOLE ? DSKY_LOAD_REQUEST_HOLE : request==DSKY_REQUEST_END ? DSKY_LOAD_REQUEST_END : request==DSKY_REQUEST_NVSUB_END ? DSKY_LOAD_REQUEST_NVSUB : DSKY_LOAD_REQUEST_PASTE;return P66_BIAS_OK;
}
enum p66_bias_status dsky_load_request(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_load_request_route *route){if(!route||!domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0,route);}
enum exec_status cm_load_request(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_load_request_route *route){dsky_lamp_io p;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;if(!domain(s,&p,1))return EXEC_OUT_OF_SCOPE;if(body(s,&p,io,1,route)!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
