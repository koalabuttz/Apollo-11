/* SPDX-License-Identifier: MIT */
#include "dsky_lamp_lifecycle.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
static int resource(const uint16_t *w,uint8_t cm,uint8_t idle){uint8_t i,cores=cm ? 7u : 8u;
 if((w[0167]==077777u&&(!idle||w[067]!=077777u))||w[0167]==0u||(w[0167]>=040000u&&w[0167]!=077777u)||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i)if(w[0167u+12u*i]==0)return 0;
 for(i=0;i<5u;++i)if(w[0400u+i*44u]&&w[0400u+i*44u]!=(uint16_t)(0400u+i*44u))return 0;
 return 1;
}
static int wake(const uint16_t *w,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u,found=0;uint16_t list=w[01043];if(!list)return 1;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(!found&&p>=040000u&&p<077777u&&(loc==list||((loc==0||loc==077777u)&&(list==0||list==077777u)))){if(w[0165u+12u*i]&077610u)return 0;found=1;}}
 return 1;
}
static int domain(const apollo_overlay_state *s,const dsky_lamp_io *io,const cm_native_io *cmio,uint8_t cm){const uint16_t *w;uint16_t pc;uint8_t on;
 if(!s||!io)return 0;
 w=s->words;pc=w[5];on=(uint8_t)(pc==03613u||pc==(cm ? 03633u : 03631u));
 if((!on&&pc!=03662u&&pc!=03667u)||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->channel12>077777u||io->channel13>077777u||(io->channel13&034000u)||io->restart_light>1u)return 0;
 if(pc==(cm ? 03633u : 03631u)&&w[0]>10u)return 0;
 if(cm&&pc==03613u&&w[01011]!=0u&&w[01011]!=077777u)return 1;
 if(on)return w[026]<=037777u&&(!cm||cmio->waitlist_alarm==0u);
 if(!resource(w,cm,(uint8_t)(pc==03662u))||(cm&&cmio->executive_alarm))return 0;
 return pc==03662u||wake(w,cm);
}
static enum p66_bias_status body(apollo_overlay_state *s,dsky_lamp_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_lamp_lifecycle_route *route){enum dsky_lamp_route control;enum dsky_mode_admit_route mode;enum dsky_lamp_release_route release;enum exec_status admitted;uint16_t *w=s->words;exec_address pair;
 if(cm){if(cm_lamp_test(s,cmio,&io->wrote_channels,&io->restart_light,&control)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;io->channel13=cmio->channel13;}
 else if(dsky_lamp_test(s,io,&control)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;
 if(control==DSKY_LAMP_PIN){*route=DSKY_LAMP_LIFECYCLE_PIN;return P66_BIAS_OK;}
 if(control==DSKY_LAMP_WAIT){pair.location=03662;pair.bank=062102;
  if(cm){enum cm_wl_status status=cm_waitlist_extended(s,cmio,pair,0);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;if(status==CM_WL_FULL){*route=DSKY_LAMP_LIFECYCLE_WAIT_ALARM;return P66_BIAS_OK;}if(status!=CM_WL_OK)return P66_BIAS_OUT_OF_SCOPE;}
  else{enum wl_status status=lm_waitlist_image(s,&io->control,LM_WAITLIST_ENTRY,pair);if(status==WL_FULL){*route=DSKY_LAMP_LIFECYCLE_WAIT_ALARM;return P66_BIAS_OK;}if(status!=WL_OK)return P66_BIAS_OUT_OF_SCOPE;}
  w[2]=03654;w[5]=cm ? 05217u : 05155u;*route=DSKY_LAMP_LIFECYCLE_END;return P66_BIAS_OK;
 }
 if(control==DSKY_LAMP_ADMIT){pair.location=03667;pair.bank=062102;if(cm){admitted=cm_admission_complete(s,cmio,CM_ADMIT_NOVAC,pair);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}else admitted=lm_admission_image(s,&io->control,LM_ADMIT_NOVAC,pair);if(admitted==EXEC_NO_CORE){*route=DSKY_LAMP_LIFECYCLE_ADMISSION_ALARM;return P66_BIAS_OK;}if(admitted!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;w[2]=03667;w[5]=cm ? 05324u : 05261u;*route=DSKY_LAMP_LIFECYCLE_TASKOVER;return P66_BIAS_OK;}
 if(cm){if(cm_mode_admit(s,cmio,&mode)!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}else if(dsky_mode_admit(s,&io->control,&mode)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;
 if(mode==DSKY_MODE_ADMIT_ALARM){*route=DSKY_LAMP_LIFECYCLE_ADMISSION_ALARM;return P66_BIAS_OK;}
 if(cm){(void)cm_interbank_complete(s,cmio,CM_IC_SWRETURN,0,0);(void)cm_lamp_test(s,cmio,&io->wrote_channels,&io->restart_light,&control);(void)cm_lamp_release(s,cmio,&io->wrote_channels,&io->restart_light,&release);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;io->channel11=cmio->channel11;io->channel13=cmio->channel13;}
 else{(void)lm_interbank(s,&io->control,LM_IB_SWRETURN,0,0);(void)dsky_lamp_test(s,io,&control);(void)dsky_lamp_release(s,io,&release);}
 *route=release==DSKY_LAMP_RELEASE_END ? DSKY_LAMP_LIFECYCLE_END : release==DSKY_LAMP_RELEASE_PIN ? DSKY_LAMP_LIFECYCLE_PIN : DSKY_LAMP_LIFECYCLE_BLANK;return P66_BIAS_OK;
}
enum p66_bias_status dsky_lamp_lifecycle(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_lamp_lifecycle_route *route){if(!route||!domain(s,io,0,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0,route);}
enum exec_status cm_lamp_lifecycle(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_lamp_lifecycle_route *route){dsky_lamp_io p;enum p66_bias_status status;if(!io||!event||!restart||!route)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;if(!domain(s,&p,io,1))return EXEC_OUT_OF_SCOPE;status=body(s,&p,io,1,route);if(status)return EXEC_OUT_OF_SCOPE;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
