/* SPDX-License-Identifier: MIT */
#include "dsky_pinbranch.h"
static enum p66_bias_status pinbranch(apollo_overlay_state *s,p66_bias_io *io,enum dsky_pinbranch_route *route,uint8_t cm){uint16_t *w=s->words,masked,oldflag;
 if(route==0||w[5]!=(cm ? 03272u : 03050u)||w[4]!=020000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(020000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 io->interrupt_enabled=1;w[1]=agc_extend(agc_correct(w[1]));w[0156]=w[cm ? 01072u : 01073u];masked=(uint16_t)(w[0100]&070000u);w[0]=agc_diminished_abs(masked);
 if(masked==0){w[5]=cm ? 03713u : 03471u;*route=DSKY_PINBRANCH_ERASE;return P66_BIAS_OK;}
 if(masked&040000u){w[5]=cm ? 02624u : 02377u;*route=DSKY_PINBRANCH_MARK;return P66_BIAS_OK;}
 /* Inline PINBRFLG=0105 names FLAGWRD4 bit6. UPFLAG's DEBIT
  * quotient/remainder are4/9, and LXCH saves the previous flagword. */
 oldflag=w[0100];w[0100]=(uint16_t)(oldflag|040u);w[061]=4;w[062]=9;w[063]=oldflag;w[1]=cm ? 03305u : 03063u;w[2]=2;masked=(uint16_t)(w[0100]&020000u);w[0]=agc_diminished_abs(masked);
 if(masked){w[5]=cm ? 02766u : 02544u;*route=DSKY_PINBRANCH_PRIORITY;}else{w[5]=cm ? 03037u : 02615u;*route=DSKY_PINBRANCH_NORMAL;}return P66_BIAS_OK;
}
enum p66_bias_status dsky_pinbranch(apollo_overlay_state *s,p66_bias_io *io,enum dsky_pinbranch_route *route){return pinbranch(s,io,route,0);}
enum exec_status cm_pinbranch(apollo_overlay_state *s,cm_native_io *io,enum dsky_pinbranch_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=pinbranch(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}

static enum p66_bias_status pinbranch_resume(apollo_overlay_state *s,p66_bias_io *io,enum dsky_pinbranch_route *route,uint8_t cm){uint16_t *w=s->words,which,index;
 if(route==0||w[4]!=020000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(020000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 if(w[5]==(cm ? 03713u : 03471u))which=0;else if(w[5]==(cm ? 02624u : 02377u))which=1;else if(w[5]==(cm ? 02766u : 02544u))which=2;else if(w[5]==(cm ? 03037u : 02615u))which=3;else return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));*route=(enum dsky_pinbranch_route)which;
 if(which==0){w[0]=agc_extend(077774);w[2]=cm ? 03715u : 03473u;w[5]=cm ? 04170u : 04154u;return P66_BIAS_OK;}
 if(which==1){w[0100]=(uint16_t)((w[0100]&077772u)+1u);io->interrupt_enabled=1;if(w[cm ? 01070u : 01071u]&020u)w[0370]=agc_negate(w[0370]);index=1;}else index=which==2u ? 0u : 2u;
 w[0]=index;w[0164]=index;w[5]=cm ? 03312u : 03070u;return P66_BIAS_OK;
}
enum p66_bias_status dsky_pinbranch_resume(apollo_overlay_state *s,p66_bias_io *io,enum dsky_pinbranch_route *route){return pinbranch_resume(s,io,route,0);}
enum exec_status cm_pinbranch_resume(apollo_overlay_state *s,cm_native_io *io,enum dsky_pinbranch_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=pinbranch_resume(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
