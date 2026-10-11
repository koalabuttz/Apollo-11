/* SPDX-License-Identifier: MIT; original PRIOLARM/PRIOENT+1 controls. */
#include "cm_priolarm.h"
static void lxch(agc_word *w,unsigned address){agc_word old=w[address];w[address]=agc_correct(w[1]);w[1]=agc_extend(old);}
enum cm_priolarm_result cm_priolarm(apollo_overlay_state *s,cm_native_io *io){agc_word *w,q;agc_register a;enum agc_ccs_class first,second;unsigned lamp=0;int ov;
 if(!s||!io)return CM_PRIOLARM_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=03766u||w[4]!=020000u||w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(020000u|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return CM_PRIOLARM_OUT_OF_SCOPE;
 io->interrupt_enabled=0;ov=agc_overflow(w[0]);w[1]=agc_extend(agc_correct(w[0]));w[01363]=ov?(agc_word)(ov>0?1u:077776u):w[0133];q=03774;
 w[2]=q;w[01364]=(agc_word)(w[0134]|io->superbank);w[061]=q;w[057]=w[0164];w[060]=w[0165];
 first=agc_classify(w[0375]);a=agc_extend(agc_diminished_abs(w[0375]));
 if(first==AGC_PLUS_ZERO||first==AGC_NEGATIVE){if(first==AGC_PLUS_ZERO)lxch(w,0375);lamp=1;}
 else{second=agc_classify(w[0376]);a=agc_extend(agc_diminished_abs(w[0376]));
  if(second==AGC_PLUS_ZERO||second==AGC_NEGATIVE){if(second==AGC_PLUS_ZERO)lxch(w,0376);}
  else{agc_word masked=(agc_word)(w[0377]&037777u);a=agc_extend(agc_diminished_abs(masked));if(!masked)lxch(w,0377);else{a=agc_add_register(w[1],agc_extend(040000));w[0377]=agc_correct(a);if(agc_overflow(a)){w[0]=agc_overflow(a)>0?1u:0177776u;w[5]=05721;return CM_PRIOLARM_POODOO;}}}
 }
 if(lamp){a=agc_add_register(agc_extend((agc_word)((agc_negate(w[01036]))&040400u)),agc_extend(w[01036]));w[01036]=agc_correct(a);}
 w[061]=agc_correct(a);w[0]=01211;io->interrupt_enabled=1;w[2]=05715;w[5]=02726;return CM_PRIOLARM_DISPLAY;
}
