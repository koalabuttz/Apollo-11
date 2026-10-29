/* SPDX-License-Identifier: MIT; actual Comanche priority display controls. */
#include "cm_priority_display.h"
#include "cm_admission_zero.h"
#include <stdbool.h>
static bool context(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&s->words[4]==020000u&&s->words[3]<=03400u&&!(s->words[3]&0377u)&&s->words[6]==(agc_word)(020000u|(s->words[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;}
static agc_register cadr(agc_word *w){return agc_add_register(agc_extend((agc_word)(w[0133]&01777u)),agc_extend(w[0134]));}
enum exec_status cm_priority_display_step(apollo_overlay_state *s,cm_native_io *io,unsigned group){agc_word *w,loc,old0,old1;agc_register a;int ov;
 if(!context(s,io)||group>2u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(group==0u){if(w[5]!=02726u||agc_overflow(w[0])||!(w[0167]&037000u))return EXEC_OUT_OF_SCOPE;
  w[0155]=agc_correct(w[0]);w[0160]=0110;io->interrupt_enabled=0;w[072]=3;w[063]=(agc_word)(w[0167]&037000u);w[1]=agc_extend((agc_word)(w[6]|io->superbank));w[0]=02770;w[2]=03107;w[5]=05160;return EXEC_OK;
 }
 if(group==1u)return cm_admission_zero(s,io,CM_ADMIT_SPVAC,(exec_address){0,0});
 if(w[5]!=03107u||io->interrupt_enabled||w[064]>72u||w[064]%12u||w[0133]<0100u||w[0133]>07777u||w[0134]>076000u||(w[0134]&01777u))return EXEC_OUT_OF_SCOPE;
 loc=w[064];w[0160]=(agc_word)((w[0160]&074377u)+w[3]);a=cadr(w);w[0157]=agc_correct(a);
 /* DCA/DXCH retains the source outgoing words while copying display input
  * into the newly admitted original core; all reads precede each pair write. */
 old0=w[0155u+loc];old1=w[0156u+loc];w[0155u+loc]=w[0155];w[0156u+loc]=w[0156];w[0]=agc_extend(old0);w[1]=agc_extend(old1);
 old0=w[0157u+loc];old1=w[0160u+loc];w[0157u+loc]=w[0157];w[0160u+loc]=w[0160];w[0]=agc_extend(old0);w[1]=agc_extend(old1);
 w[0161]=loc;a=cadr(w);w[0157]=agc_correct(a);a=agc_add_register(a,agc_extend(w[072]));w[1]=03123;w[2]=2;io->interrupt_enabled=1;
 /* The internal source TC L has Q=2, so execute this fixed BANKJUMP control
  * directly rather than imposing a public ordinary-caller Q guard. */
 w[4]=(agc_word)(agc_correct(a)&076000u);w[6]=(agc_word)(w[4]|(w[3]>>8));ov=agc_overflow(a);w[0]=2;
 if(ov<0){agc_word operand=w[w[3]+0377u];enum agc_ccs_class cl=agc_classify(operand);w[0]=agc_diminished_abs(operand);if(cl==AGC_POSITIVE)w[0]=agc_extend((agc_word)(w[0133]&01777u));else if(cl==AGC_PLUS_ZERO)w[0]=0;if(cl!=AGC_MINUS_ZERO)w[0]=agc_add_register(w[0],agc_extend(w[0134]));w[2]=0177776;w[5]=2;}
 else if(ov>0){w[2]=1;w[5]=02001;}else{w[2]=(agc_word)(agc_correct(a)&01777u);w[5]=(agc_word)(02000u+w[2]);}
 return EXEC_OK;
}
