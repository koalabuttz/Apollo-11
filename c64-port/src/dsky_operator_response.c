/* SPDX-License-Identifier: MIT */
#include "dsky_operator_response.h"
#include "dsky_display_release.h"
static int equal(uint16_t a,uint16_t b){return a==b||((a==0u||a==077777u)&&(b==0u||b==077777u));}
static int wake_domain(const uint16_t *w,uint8_t cm,uint16_t cadr,uint8_t skip,uint8_t *slot){uint8_t i,cores=cm ? 7u : 8u;*slot=cores;if(w[0167]==0u||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i];if(p==0u)return 0;if(i!=skip&&*slot==cores&&p>=040000u&&p<077777u&&equal(w[0164u+12u*i],cadr)){if(w[0165u+12u*i]&077610u)return 0;*slot=i;}}return 1;}
static int entry(uint16_t pc,uint8_t cm){static const uint16_t lm[]={03442,03443,03450,03457,03452},c[]={03347,03350,03355,03364,03357};uint8_t i;for(i=0;i<5u;++i)if(pc==(cm ? c[i] : lm[i]))return i;return -1;}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,const cm_native_io *cmio,uint8_t cm){const uint16_t *w;uint8_t slot=255u,second;
 if(!s||!io)return 0;
 w=s->words;if(entry(w[5],cm)<0||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->restart_light>1u)return 0;
 if(w[01043]!=0u&&((cm&&cmio->executive_alarm)||!wake_domain(w,cm,w[01043],255u,&slot)))return 0;
 if(w[01042]!=0u&&((cm&&cmio->executive_alarm)||!wake_domain(w,cm,w[01042],slot,&second)))return 0;
 return 1;}
static enum p66_bias_status body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm){uint16_t *w=s->words,start=cm ? 03347u : 03442u;int e=entry(w[5],cm);uint8_t overflow;dsky_charin_io p;
 w[1]=agc_extend(agc_correct(w[1]));if(e==4){w[01013]=0;w[0777]=077754;w[0]=1;}else if(e==0)w[0]=1;else if(e==2)w[0]=0177776;else if(e==3)w[0]=0177777;
 w[01014]=agc_correct(w[0]);overflow=(uint8_t)((w[0]&0140000u)==040000u||(w[0]&0140000u)==0100000u);
 /* A raw overflow at VBPROC+1 skips the actual TC KILMONON. */
 if(!overflow)w[01021]=040000;
 w[2]=(uint16_t)(start+4u);w[5]=cm ? 04473u : 04457u;
 if(cm){uint16_t event=0;if(cm_display_release(s,cmio,&event)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;}
 else{p.control=io->control;p.channel11=io->channel11;p.wrote_channel11=0;if(dsky_display_release(s,&p)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;io->control=p.control;io->channel11=p.channel11;}
 io->channel11=(uint16_t)(io->channel11&077737u);io->wrote_channels=(uint16_t)(io->wrote_channels|1u);if(io->channel11&01000u)io->restart_light=0;w[0]=agc_extend(io->channel11);w[2]=(uint16_t)(start+6u);w[5]=cm ? 03454u : 03547u;
 if(cm){cmio->channel11=io->channel11;if(cm_response_recall(s,cmio,&io->wrote_channels,&io->restart_light)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;return P66_BIAS_OK;}
 return dsky_response_recall(s,io);
}
enum p66_bias_status dsky_operator_response(apollo_overlay_state *s,dsky_lamp_io *io){if(!domain(s,io,0,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0);}
enum exec_status cm_operator_response(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart){dsky_lamp_io p;if(!io||!event||!restart)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;if(!domain(s,&p,io,1))return EXEC_OUT_OF_SCOPE;if(body(s,&p,io,1)!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
