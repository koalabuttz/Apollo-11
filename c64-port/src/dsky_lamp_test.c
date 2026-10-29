/* SPDX-License-Identifier: MIT */
#include "dsky_lamp_test.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
#include "dsky_source_alarm.h"
static void channel11(dsky_lamp_io *io,uint16_t value){io->channel11=value;io->wrote_channels=(uint16_t)(io->wrote_channels|1u);if(value&01000u)io->restart_light=0;}
static void cmio(const dsky_lamp_io *io,cm_native_io *p){p->superbank=io->control.superbank;p->interrupt_enabled=io->control.interrupt_enabled;p->channel11=io->channel11;p->channel12=io->channel12;p->channel13=io->channel13;}
static int interbank(apollo_overlay_state *s,dsky_lamp_io *io,uint8_t cm,uint8_t kind,uint16_t literal){
 if(cm){cm_native_io p={0};int ok;cmio(io,&p);s->words[5]=kind==0u ? 04662u : kind==1u ? 04675u : 04701u;ok=cm_interbank_complete(s,&p,kind==0u ? CM_IC_BANKCALL : kind==1u ? CM_IC_SWRETURN : CM_IC_POSTJUMP,literal,0);io->control.superbank=p.superbank;io->control.interrupt_enabled=p.interrupt_enabled;return ok;}
 s->words[5]=kind==0u ? 04616u : kind==1u ? 04631u : 04635u;return lm_interbank(s,&io->control,kind==0u ? LM_IB_BANKCALL : kind==1u ? LM_IB_SWRETURN : LM_IB_POSTJUMP,literal,0);
}
static void chkpooh(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_lamp_route *route){uint16_t *w=s->words;
 w[0]=agc_extend(w[01011]);
 if((w[01011]==0u||w[01011]==077777u)){uint16_t caller=w[2];w[2]=3;w[5]=caller;*route=DSKY_LAMP_CALLER;}
 else{cm_native_io p={0};enum dsky_alarm_route r=DSKY_ALARM_PINBRANCH;uint16_t event=0;cmio(io,&p);w[5]=02120;(void)cm_source_alarm(s,&p,&event,&r);io->channel11=p.channel11;io->control.superbank=p.superbank;io->control.interrupt_enabled=p.interrupt_enabled;channel11(io,p.channel11);*route=DSKY_LAMP_PIN;}
}
static enum p66_bias_status lamp(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_lamp_route *route,uint8_t cm){uint16_t *w,pc,count,i,mode33=cm ? 01321u : 01303u,mode30=cm ? 01320u : 01302u;
 if(!s||!io||!route)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;pc=w[5];
 if(w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=(cm&&pc==02730u ? 066000u : 062000u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>077777u||io->channel12>077777u||io->channel13>077777u||(io->channel13&034000u)||io->restart_light>1u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc!=03613u&&pc!=(cm ? 03633u : 03631u)&&pc!=03662u&&pc!=03667u&&pc!=(cm ? 03721u : 03722u)&&!(cm&&pc==02730u))return P66_BIAS_OUT_OF_SCOPE;
 if(pc==(cm ? 03633u : 03631u)&&w[0]>10u)return P66_BIAS_OUT_OF_SCOPE;
 if(cm&&pc==02730u&&(w[01011]==0u||w[01011]==077777u)&&(w[2]<0100u||w[2]>07777u))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(cm&&pc==02730u){chkpooh(s,io,route);return P66_BIAS_OK;}
 if(pc==03613u&&cm){w[2]=03614;(void)interbank(s,io,1,0,066730);chkpooh(s,io,route);if(*route==DSKY_LAMP_PIN)return P66_BIAS_OK;/* CHKPOOH's actual TC Q leaves Q=3, outside the unchanged generic
   * SWRETURN helper caller guard. Translate the literal source SWRETURN
   * exchanges here without manufacturing a different incoming Q. */
  {uint16_t oldfb=w[4];w[4]=(uint16_t)(w[0134]&076000u);w[6]=(uint16_t)(w[4]|(w[3]>>8));w[0134]=oldfb;w[0]=agc_extend(agc_correct(w[0]));w[2]=0134;w[5]=w[0133];}}
 if(pc==03613u){io->control.interrupt_enabled=0;w[mode33]=(uint16_t)(w[mode33]|1u);channel11(io,(uint16_t)(io->channel11|0175u));w[01036]=cm ? 040650u : 040674u;io->channel13=(uint16_t)(io->channel13|01000u);io->wrote_channels=(uint16_t)(io->wrote_channels|2u);count=10;}
 else count=w[0];
 if(pc==03613u||pc==(cm ? 03633u : 03631u)){for(i=0;i<=count;++i)w[01023u+i]=072102;w[0117]=0;w[01024]=w[01027]=w[01031]=070102;w[01016]=11;w[0]=00764;w[2]=03651;w[5]=cm ? 05245u : 05203u;if(!cm)io->control.interrupt_enabled=0;*route=DSKY_LAMP_WAIT;return P66_BIAS_OK;}
 if(pc==03662u){w[0]=030000;w[2]=03664;w[5]=cm ? 05134u : 05072u;*route=DSKY_LAMP_ADMIT;return P66_BIAS_OK;}
 if(pc==03667u){io->control.interrupt_enabled=0;channel11(io,(uint16_t)(io->channel11&077662u));io->channel13=(uint16_t)(io->channel13&076777u);io->wrote_channels=(uint16_t)(io->wrote_channels|2u);w[01036]=(uint16_t)((io->channel12&010u)+040000u);w[mode33]=(uint16_t)((w[mode33]&061776u)|016000u);w[mode30]=(uint16_t)((w[mode30]&022777u)|015000u);
  if(cm){w[0]=agc_add_register(agc_extend((uint16_t)(agc_negate(w[01331])&0100u)),agc_extend(w[01331]));w[01331]=agc_correct(w[0]);}
  else{w[0110]=(uint16_t)((w[0110]&077447u)|0100u);w[0]=agc_extend(w[0110]);}
  io->control.interrupt_enabled=1;w[2]=cm ? 03720u : 03721u;(void)interbank(s,io,cm,0,cm ? 010566u : 010636u);*route=DSKY_LAMP_MODE;return P66_BIAS_OK;
 }
 /* Actual KILMONON is an overwrite, not a read/modify/set of MONSAVE1. */
 w[01021]=040000;channel11(io,(uint16_t)(io->channel11&077737u));w[0]=agc_extend(io->channel11);w[2]=cm ? 03724u : 03725u;(void)interbank(s,io,cm,2,cm ? 061376u : 061471u);*route=DSKY_LAMP_RELEASE;return P66_BIAS_OK;
}
enum p66_bias_status dsky_lamp_test(apollo_overlay_state *s,dsky_lamp_io *io,enum dsky_lamp_route *route){return lamp(s,io,route,0);}
enum exec_status cm_lamp_test(apollo_overlay_state *s,cm_native_io *io,uint16_t *event,uint16_t *restart,enum dsky_lamp_route *route){dsky_lamp_io p;enum p66_bias_status status;if(!io||!event||!restart)return EXEC_OUT_OF_SCOPE;p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.channel12=io->channel12;p.channel13=io->channel13;p.wrote_channels=*event;p.restart_light=*restart;status=lamp(s,&p,route,1);if(status)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;io->channel12=p.channel12;io->channel13=p.channel13;*event=p.wrote_channels;*restart=p.restart_light;return EXEC_OK;}
