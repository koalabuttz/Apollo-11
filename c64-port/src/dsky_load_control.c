/* SPDX-License-Identifier: MIT */
#include "dsky_load_control.h"
#include "lm_basic_wake.h"
#include "cm_basic_wake.h"
static int wake_domain(const uint16_t *w,uint8_t cm,uint16_t wake){uint8_t i,cores=cm ? 7u : 8u,found=0;
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;
  if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if((w[0165u+12u*i]&077610u)!=0)return 0;found=1;}}
 return 1;
}
static enum p66_bias_status release(apollo_overlay_state *s,dsky_charin_io *io,cm_native_io *cmio,uint8_t cm,uint16_t caller){enum exec_status status;s->words[2]=caller;s->words[5]=cm ? 04473u : 04457u;if(!cm)return dsky_display_release(s,io);status=cm_display_release(s,cmio,&io->wrote_channel11);if(status!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;return P66_BIAS_OK;}
static enum p66_bias_status control_body(apollo_overlay_state *s,dsky_charin_io *io,cm_native_io *cmio,uint8_t cm){uint16_t *w=s->words,pc=w[5],base=cm ? 03347u : 03442u,recall=cm ? 03454u : 03547u,cadr=w[01042],list=w[01043],offset,address,low,oldhigh,oldlow,delta,load;agc_register sum;int overflow;enum exec_status status;uint8_t direct=(uint8_t)(pc==recall),prockey=(uint8_t)(pc==(cm ? 03357u : 03452u));
 if(!(direct||prockey||pc==base||pc==(cm ? 03355u : 03450u)||pc==(cm ? 03364u : 03457u))||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||w[7]!=0||io->control.superbank>0160u||(io->control.superbank&017u)||(io->control.superbank&0100u)==0||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(cm&&cmio->executive_alarm!=0&&((!direct&&list!=0)||cadr!=0))return P66_BIAS_OUT_OF_SCOPE;
 if(list!=0&&(!direct||(cadr!=0&&w[01014]!=0))&&!wake_domain(w,cm,list))return P66_BIAS_OUT_OF_SCOPE;
 if(cadr!=0&&!wake_domain(w,cm,cadr))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(!direct){w[01014]=(pc==base||prockey) ? 1u : pc==(cm ? 03355u : 03450u) ? 077776u : 077777u;if(prockey){w[01013]=0;w[0777]=077754;}w[01021]=040000;w[0]=0140000;
  if(release(s,io,cmio,cm,(uint16_t)(base+4u))!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;
  io->channel11=(uint16_t)(io->channel11&077737u);io->wrote_channel11=1;w[0]=agc_extend(io->channel11);if(cm)cmio->channel11=io->channel11;w[2]=(uint16_t)(base+6u);w[5]=recall;
 }
 if(cadr==0){w[0]=0;w[2]=(uint16_t)(recall+3u);w[5]=cm ? 05217u : 05155u;return P66_BIAS_OK;}
 w[01042]=0;w[0]=agc_extend(cadr);w[2]=(uint16_t)(recall+7u);w[5]=cm ? 05201u : 05137u;io->control.interrupt_enabled=0;
 if(cm){cmio->interrupt_enabled=0;status=cm_basic_wake(s,cmio);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}else status=lm_basic_wake(s,&io->control);
 if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND)return P66_BIAS_OUT_OF_SCOPE;
 load=w[01014];if(load==0){w[0]=0;w[2]=(uint16_t)(recall+10u);w[5]=cm ? 05217u : 05155u;return P66_BIAS_OK;}
 delta=(uint16_t)(load==077777u ? 2u : (load&040000u)!=0 ? 0u : 1u);offset=w[064];address=(uint16_t)(agc_add_word(0164,offset)&01777u);sum=agc_add_register(agc_extend(delta),agc_extend(w[address]));overflow=agc_overflow(sum);w[address]=agc_correct(sum);
 w[1]=(uint16_t)(overflow>0 ? 1u : overflow<0 ? 0177776u : agc_extend(w[01002]));w[0]=agc_extend(w[01001]);address=(uint16_t)(agc_add_word(0154,offset)&01777u);low=(uint16_t)(address+1u);oldhigh=w[address];oldlow=w[low];w[address]=agc_correct(w[0]);w[low]=agc_correct(w[1]);w[0]=agc_extend(oldhigh);w[1]=agc_extend(oldlow);io->control.interrupt_enabled=1;if(cm)cmio->interrupt_enabled=1;
 if(release(s,io,cmio,cm,(uint16_t)(recall+23u))!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;
 w[2]=(uint16_t)(recall+24u);w[5]=cm ? 05217u : 05155u;return P66_BIAS_OK;
}
enum p66_bias_status dsky_load_control(apollo_overlay_state *s,dsky_charin_io *io){return control_body(s,io,0,0);}
enum exec_status cm_load_control(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote){dsky_charin_io p;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=control_body(s,&p,io,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;}
