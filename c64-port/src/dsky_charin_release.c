/* SPDX-License-Identifier: MIT */
#include "dsky_charin_release.h"
#include "lm_basic_wake.h"
#include "cm_basic_wake.h"
#include "agc_multiword.h"
static int wake_domain(const uint16_t *w,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u,found=0;uint16_t wake=w[01043];
 if(w[0167]==0||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return 0;
  if(!found&&p>=040000u&&p<077777u&&(loc==wake||((loc==0||loc==077777u)&&(wake==0||wake==077777u)))){if((w[0165u+12u*i]&077610u)!=0)return 0;found=1;}}
 return 1;
}
static enum p66_bias_status release_body(apollo_overlay_state *s,dsky_charin_io *io,cm_native_io *cmio,enum dsky_release_route *route,uint8_t cm){
 uint16_t *w=s->words,cadr=w[01042],oldlock=w[01012],list=w[01043],unstart=cm ? 03404u : 03477u,normal=cm ? 03376u : 03471u;uint8_t unsuspend=(uint8_t)(oldlock==1u&&(w[01021]&020000u)!=0);
 if(route==0||w[0154]!=031u||w[5]!=(cm ? 02000u : 02077u)||oldlock>1u||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(060000u|(w[3]>>8))||io->control.superbank!=0100u||io->control.interrupt_enabled>1u||io->channel11>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(!unsuspend&&list!=0&&(!wake_domain(w,cm)||(cm&&cmio->executive_alarm!=0)))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=oldlock;w[01012]=1;w[7]=0;
 if(cadr!=0){io->channel11=(uint16_t)(io->channel11|020u);w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=031;io->channel11=(uint16_t)(io->channel11&077773u);io->wrote_channel11=1;
 if(!unsuspend){enum exec_status status;
  w[0144]=(uint16_t)(normal+1u);w[01021]=(uint16_t)(w[01021]&057777u);
  if(list!=0){w[01043]=0;w[0]=agc_extend(list);w[2]=cm ? 04507u : 04473u;w[5]=cm ? 05201u : 05137u;
   if(cm){status=cm_basic_wake(s,cmio);io->control.superbank=cmio->superbank;io->control.interrupt_enabled=cmio->interrupt_enabled;}
   else status=lm_basic_wake(s,&io->control);
   if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND)return P66_BIAS_OUT_OF_SCOPE;
  }
  io->control.interrupt_enabled=1;io->channel11=(uint16_t)(io->channel11&077757u);w[01012]=0;
  if(cadr==0){w[0]=0;w[2]=(uint16_t)(normal+4u);*route=DSKY_RELEASE_END;}
  else if(cadr!=077777u){w[0]=agc_extend(agc_diminished_abs(cadr));w[4]=020000;w[6]=(uint16_t)(020000u|(w[3]>>8));w[2]=cm ? 01272u : 01050u;w[5]=cm ? 03272u : 03050u;*route=DSKY_RELEASE_PINBRANCH;return P66_BIAS_OK;}
  else{
   /* The -0 CCS skip executes the inline CADR word as its fixed DAS
    * instruction, then falls into UNSUSPEN. This is a compiled exact
    * source path, never a runtime instruction decoder or rope fetch. */
   uint16_t low=cm ? 01272u : 01050u;agc_dp pair;agc_register_pair r;
   pair.high=w[low-1u];pair.low=w[low];r.a=0;r.l=w[1];agc_double_add(&r,&pair);w[low-1u]=pair.high;w[low]=pair.low;w[1]=r.l;w[0]=r.a;w[2]=0145;
  }
 }
 if(unsuspend||cadr==077777u){w[01012]=0;
  if(cadr==077777u){w[0]=0;if(unsuspend)w[2]=cm ? 03376u : 03471u;w[5]=cm ? 03412u : 03505u;*route=DSKY_RELEASE_BLANK;return P66_BIAS_OK;}
  if(cadr!=0){w[0]=agc_extend(agc_diminished_abs(cadr));w[2]=(uint16_t)(unstart+((cadr&040000u)!=0 ? 6u : 4u));}
  else{w[0144]=(uint16_t)(unstart+5u);if(list==0){io->control.interrupt_enabled=1;io->channel11=(uint16_t)(io->channel11&077757u);}w[0]=0;w[2]=(uint16_t)(unstart+6u);}
  *route=DSKY_RELEASE_END;
 }
 w[5]=cm ? 05217u : 05155u;return P66_BIAS_OK;
}
enum p66_bias_status dsky_charin_release(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_release_route *route){return release_body(s,io,0,route,0);}
enum exec_status cm_charin_release(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_release_route *route){dsky_charin_io p;enum p66_bias_status status;
 if(wrote==0)return EXEC_OUT_OF_SCOPE;
 p.control.superbank=io->superbank;p.control.interrupt_enabled=io->interrupt_enabled;p.channel11=io->channel11;p.wrote_channel11=0;status=release_body(s,&p,io,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.control.superbank;io->interrupt_enabled=p.control.interrupt_enabled;io->channel11=p.channel11;*wrote=p.wrote_channel11;return EXEC_OK;
}
