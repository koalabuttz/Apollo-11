/* SPDX-License-Identifier: MIT */
#include "dsky_load_complete.h"
#include "dsky_display_release.h"
static int dsky_load_equal(uint16_t a,uint16_t b){return a==b||((a==0u||a==077777u)&&(b==0u||b==077777u));}
static int dsky_load_wake(const uint16_t *w,uint8_t cm,uint16_t wake,uint8_t skip,uint8_t *slot){uint8_t i,cores=cm ? 7u : 8u;*slot=255u;if(w[0167]==0u||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i];if(!p)return 0;if(i!=skip&&*slot==255u&&p>=040000u&&p<077777u&&dsky_load_equal(w[0164u+12u*i],wake)){if(w[0165u+12u*i]&077610u)return 0;*slot=i;}}return 1;}
static int dsky_load_domain(const apollo_overlay_state *s,const dsky_lamp_io *io,const cm_native_io *cmio,uint8_t cm){const uint16_t *w;uint8_t listslot=255u,cadrslot;if(!s||!io)return 0;w=s->words;if(w[5]!=02775u||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->restart_light>1u)return 0;
 if((w[01043]||w[01042])&&cm&&cmio->executive_alarm)return 0;
 if(w[01043]&&!dsky_load_wake(w,cm,w[01043],255u,&listslot))return 0;
 if(w[01042]&&!dsky_load_wake(w,cm,w[01042],listslot,&cadrslot))return 0;
 return 1;
}
static enum p66_bias_status dsky_load_body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm){uint16_t *w=s->words;dsky_charin_io p;enum p66_bias_status result;w[01000]=0;w[01014]=077777;w[0]=0177777;w[2]=03002;w[5]=cm ? 04473u : 04457u;if(cm){uint16_t event=0;if(cm_display_release(s,cmio,&event)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;}else{p.control=io->control;p.channel11=io->channel11;p.wrote_channel11=0;result=dsky_display_release(s,&p);if(result!=P66_BIAS_OK)return result;io->control=p.control;io->channel11=p.channel11;}io->wrote_channels=(uint16_t)(io->wrote_channels|1u);if(io->channel11&01000u)io->restart_light=0;
 w[0]=agc_extend(077754);w[0777]=077754;w[4]=060000;w[6]=(uint16_t)(060000u|(w[3]>>8));w[5]=cm ? 03454u : 03547u;w[2]=cm ? 01454u : 01547u;
 if(cm){if(cm_response_recall(s,cmio,&io->wrote_channels,&io->restart_light)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;return P66_BIAS_OK;}return dsky_response_recall(s,io);
}
enum p66_bias_status dsky_load_complete(apollo_overlay_state *s,dsky_lamp_io *io){if(!dsky_load_domain(s,io,0,0))return P66_BIAS_OUT_OF_SCOPE;return dsky_load_body(s,io,0,0);}
enum exec_status cm_load_complete(apollo_overlay_state *s,cm_native_io *io,uint16_t *events,uint16_t *restart){dsky_lamp_io p;if(!io||!events||!restart)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*events;p.restart_light=*restart;if(!dsky_load_domain(s,&p,io,1))return EXEC_OUT_OF_SCOPE;if(dsky_load_body(s,&p,io,1)!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;*events=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
