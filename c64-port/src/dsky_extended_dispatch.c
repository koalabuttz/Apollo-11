/* SPDX-License-Identifier: MIT */
#include "dsky_extended_dispatch.h"
#include "dsky_extended_dispatch_targets.inc"
static enum p66_bias_status extended_dispatch(apollo_overlay_state *s,p66_bias_io *io,uint8_t cm){uint16_t *w=s->words,index=w[0154],target;uint8_t entry=(uint8_t)(w[5]==02143u);
 if((!entry&&w[5]!=02000u)||w[4]!=(entry ? 062000u : 066000u)||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u||(index!=077777u&&index>59u))return P66_BIAS_OUT_OF_SCOPE;
 if(index==077777u)index=0;
 w[1]=agc_extend(agc_correct(w[1]));
 if(entry){w[4]=066000;w[6]=(uint16_t)(066000u|(w[3]>>8));}
 target=cm ? cm_extended_target(index) : lm_extended_target(index);
 w[2]=(uint16_t)((index==9u||(cm&&index==56u)) ? 02002u : 02003u+index);w[5]=target;return P66_BIAS_OK;
}
enum p66_bias_status dsky_extended_dispatch(apollo_overlay_state *s,p66_bias_io *io){return extended_dispatch(s,io,0);}
enum exec_status cm_extended_dispatch(apollo_overlay_state *s,cm_native_io *io){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=extended_dispatch(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
