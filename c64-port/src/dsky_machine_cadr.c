/* SPDX-License-Identifier: MIT */
#include "dsky_machine_cadr.h"
static enum p66_bias_status machine_cadr(apollo_overlay_state *s,p66_bias_io *io,enum dsky_machine_route *route,uint8_t cm){uint16_t *w=s->words,i;enum dsky_octal_route display_route=DSKY_OCTAL_CALLER;enum p66_bias_status status;
 if(route==0||(w[5]!=02116u&&w[5]!=02120u)||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(w[5]==02116u){w[01017]=w[0156];w[3]=(uint16_t)(w[01017]&03400u);w[6]=(uint16_t)(062000u|(w[3]>>8));w[0]=agc_extend((uint16_t)((w[01017]&0377u)+01400u));w[0145]=agc_correct(w[0]);w[2]=3;}
 w[0]=agc_add_register(agc_extend(077772u),agc_extend(w[01001]));
 if(w[0]==0||w[0]==0177777u){w[5]=02133;*route=DSKY_MACHINE_VERB_FAN;return P66_BIAS_OK;}
 w[0777]=4;w[0]=agc_extend(w[01017]);w[2]=02130;w[5]=03363;
 if(cm){cm_native_io view={0};view.superbank=io->superbank;view.interrupt_enabled=io->interrupt_enabled;status=cm_octal_display(s,&view,&display_route)==EXEC_OK ? P66_BIAS_OK : P66_BIAS_OUT_OF_SCOPE;io->superbank=view.superbank;io->interrupt_enabled=view.interrupt_enabled;}else status=dsky_octal_display(s,io,&display_route);
 if(status!=P66_BIAS_OK)return status;
 if(display_route==DSKY_OCTAL_CCS_HOLE){*route=DSKY_MACHINE_CCS_HOLE;return P66_BIAS_OK;}
 w[2]=02131;w[5]=02133;*route=DSKY_MACHINE_VERB_FAN;return P66_BIAS_OK;
}
enum p66_bias_status dsky_machine_cadr(apollo_overlay_state *s,p66_bias_io *io,enum dsky_machine_route *route){return machine_cadr(s,io,route,0);}
enum exec_status cm_machine_cadr(apollo_overlay_state *s,cm_native_io *io,enum dsky_machine_route *route){p66_bias_io view;enum p66_bias_status status;view.superbank=io->superbank;view.interrupt_enabled=io->interrupt_enabled;status=machine_cadr(s,&view,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=view.superbank;io->interrupt_enabled=view.interrupt_enabled;return EXEC_OK;}
