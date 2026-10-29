/* SPDX-License-Identifier: MIT */
#include "dsky_jam_control.h"
static int wake_domain(const uint16_t *w,uint8_t cm,uint16_t wake){uint8_t i,cores=cm ? 7u : 8u,found=0;
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;
  if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if((w[0165u+12u*i]&077610u)!=0)return 0;found=1;}}
 return 1;
}
static enum p66_bias_status jam_body(apollo_overlay_state *s,dsky_charin_io *io,cm_native_io *cmio,uint16_t *wrote_super,uint8_t cm){uint16_t *w=s->words,cadr=w[01042],list=w[01043],pc=w[5];enum p66_bias_status result;enum exec_status status;uint8_t term=(uint8_t)(pc==(cm ? 04245u : 04231u));
 if(wrote_super==0||!(term||pc==(cm ? 04257u : 04243u))||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if((list!=0&&!wake_domain(w,cm,list))||(cadr!=0&&!wake_domain(w,cm,cadr))||(cm&&cmio->executive_alarm!=0&&(list!=0||cadr!=0)))return P66_BIAS_OUT_OF_SCOPE;
 io->control.superbank=0100;w[01013]=term ? 34u : 33u;w[0777]=077754;w[1]=agc_extend(agc_correct(w[1]));w[0]=0177754;w[4]=060000;w[6]=(uint16_t)(060000u|(w[3]>>8));w[5]=term ? (cm ? 03355u : 03450u) : (cm ? 03347u : 03442u);w[2]=(uint16_t)(w[5]-02000u);
 if(cm){cmio->superbank=0100;status=cm_load_control(s,cmio,&io->wrote_channel11);if(status!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;result=P66_BIAS_OK;}else result=dsky_load_control(s,io);
 if(result==P66_BIAS_OK){*wrote_super=1;}
 return result;
}
enum p66_bias_status dsky_jam_control(apollo_overlay_state *s,dsky_charin_io *io,uint16_t *wrote_super){return jam_body(s,io,0,wrote_super,0);}
enum exec_status cm_jam_control(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,uint16_t *wrote_super){dsky_charin_io p;enum p66_bias_status result;if(wrote==0)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;result=jam_body(s,&p,io,wrote_super,1);if(result!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;}
