/* SPDX-License-Identifier: MIT */
#include "dsky_display_release.h"
#include "lm_basic_wake.h"
#include "cm_basic_wake.h"
static int wake_domain(const uint16_t *w,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u,found=0;uint16_t wake=w[01043];
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;
  if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if((w[0165u+12u*i]&077610u)!=0)return 0;found=1;}}
 return 1;
}
static enum p66_bias_status release_body(apollo_overlay_state *s,dsky_charin_io *io,cm_native_io *cmio,uint8_t cm){uint16_t *w=s->words,caller=w[2],list=w[01043];enum exec_status status;
 if(w[5]!=(cm ? 04473u : 04457u)||caller<0100u||caller>07777u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(list!=0&&(!wake_domain(w,cm)||(cm&&cmio->executive_alarm!=0)))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0144]=caller;w[01021]=(uint16_t)(w[01021]&057777u);
 if(list!=0){w[01043]=0;w[0]=agc_extend(list);w[2]=cm ? 04507u : 04473u;w[5]=cm ? 05201u : 05137u;
  if(cm){status=cm_basic_wake(s,cmio);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}else status=lm_basic_wake(s,&io->control);
  if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND)return P66_BIAS_OUT_OF_SCOPE;
 }
 io->control.interrupt_enabled=1;io->channel11=(uint16_t)(io->channel11&077757u);io->wrote_channel11=1;w[01012]=0;w[0]=0;w[2]=0145;w[5]=caller;return P66_BIAS_OK;
}
enum p66_bias_status dsky_display_release(apollo_overlay_state *s,dsky_charin_io *io){return release_body(s,io,0,0);}
enum exec_status cm_display_release(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io p;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=release_body(s,&p,io,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;}
