/* SPDX-License-Identifier: MIT */
#include "dsky_lamp_release.h"
#include "agc_multiword.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u,found=0;const uint16_t *w;uint16_t wake;
 if(!s||!io)return 0;
 w=s->words;wake=w[01043];
 if(w[5]!=(cm ? 03376u : 03471u)||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->restart_light>1u)return 0;
 if(!wake)return 1;
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;
  if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if((w[0165u+12u*i]&077610u)!=0)return 0;found=1;}}
 return 1;
}
static enum p66_bias_status body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_lamp_release_route *route){uint16_t *w=s->words,normal=cm ? 03376u : 03471u,cadr=w[01042];dsky_charin_io p;enum exec_status status;
 w[2]=(uint16_t)(normal+1u);w[5]=cm ? 04473u : 04457u;
 if(cm){uint16_t event=0;status=cm_display_release(s,cmio,&event);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;if(status!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;}
 else{p.control=io->control;p.channel11=io->channel11;p.wrote_channel11=0;if(dsky_display_release(s,&p)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;io->control=p.control;io->channel11=p.channel11;}
 io->wrote_channels=(uint16_t)(io->wrote_channels|1u);if(io->channel11&01000u)io->restart_light=0;
 if(cadr==0){w[0]=0;w[2]=(uint16_t)(normal+4u);w[5]=cm ? 05217u : 05155u;*route=DSKY_LAMP_RELEASE_END;return P66_BIAS_OK;}
 if(cadr==077777u){uint16_t low=cm ? 01272u : 01050u;agc_dp pair;agc_register_pair r;pair.high=w[low-1u];pair.low=w[low];r.a=0;r.l=w[1];agc_double_add(&r,&pair);w[low-1u]=pair.high;w[low]=pair.low;w[1]=r.l;w[0]=0;w[01012]=0;w[5]=cm ? 03412u : 03505u;*route=DSKY_LAMP_RELEASE_BLANK;return P66_BIAS_OK;}
 w[0]=agc_extend(agc_diminished_abs(cadr));w[2]=(uint16_t)(normal+5u);
 if(cm){w[5]=04701;(void)cm_interbank_complete(s,cmio,CM_IC_POSTJUMP,021272,0);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}
 else{w[5]=04635;(void)lm_interbank(s,&io->control,LM_IB_POSTJUMP,021050,0);}
 *route=DSKY_LAMP_RELEASE_PIN;return P66_BIAS_OK;
}
enum p66_bias_status dsky_lamp_release(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_lamp_release_route *route){if(!route||!domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0,route);}
enum exec_status cm_lamp_release(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_lamp_release_route *route){dsky_lamp_io p;enum p66_bias_status status;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;if(!domain(s,&p,1)||(s->words[01043]&&io->executive_alarm))return EXEC_OUT_OF_SCOPE;status=body(s,&p,io,1,route);if(status)return EXEC_OUT_OF_SCOPE;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
