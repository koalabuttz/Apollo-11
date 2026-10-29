/* SPDX-License-Identifier: MIT */
#include "dsky_verb_dispatch.h"
#include "dsky_verb_dispatch_table.inc"
static enum p66_bias_status verb_dispatch(apollo_overlay_state *s,p66_bias_io *io,enum dsky_verb_route *route,uint8_t cm){uint16_t *w=s->words,verb=w[01001],cadr;
 if(route==0||w[5]!=02133u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u||(verb!=077777u&&verb>99u))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(verb!=077777u&&verb>=40u){w[0154]=(uint16_t)(verb-40u);w[0]=w[0154];w[2]=02143;w[5]=cm ? 04473u : 04457u;*route=DSKY_VERB_EXTENDED_RELEASE;return P66_BIAS_OK;}
 cadr=cm ? verb_cm[verb==077777u ? 0u : verb] : verb_lm[verb==077777u ? 0u : verb];w[0]=02151;w[2]=(uint16_t)(cadr&01777u);w[4]=(uint16_t)(cadr&076000u);w[6]=(uint16_t)(w[4]|(w[3]>>8));w[5]=(uint16_t)(02000u+w[2]);*route=DSKY_VERB_DIRECT;return P66_BIAS_OK;
}
enum p66_bias_status dsky_verb_dispatch(apollo_overlay_state *s,p66_bias_io *io,enum dsky_verb_route *route){return verb_dispatch(s,io,route,0);}
enum exec_status cm_verb_dispatch(apollo_overlay_state *s,cm_native_io *io,enum dsky_verb_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=verb_dispatch(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
