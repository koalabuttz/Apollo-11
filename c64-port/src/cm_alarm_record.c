/* SPDX-License-Identifier: MIT */
#include "cm_alarm_record.h"
static void lxch(agc_word *w,unsigned address){agc_word old=w[address];w[address]=agc_correct(w[1]);w[1]=agc_extend(old);}
enum exec_status cm_alarm_record(apollo_overlay_state *s,cm_native_io *io,enum cm_alarm_record_entry entry,agc_word literal){agc_word *w,q,ret;agc_register a;enum agc_ccs_class first,second;static const agc_word pc[4]={05650,05764,05757,05655};unsigned lamp=0;
 if(!s||!io||(unsigned)entry>=4u||literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 w=s->words;q=w[2];
 if(w[5]!=pc[entry]||q<0100u||q>07775u||w[7]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if((entry==CM_RECORD_VARALARM||entry==CM_RECORD_BORTENT)&&agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
 io->interrupt_enabled=0;
 if(entry==CM_RECORD_ALARM){w[01363]=q;w[1]=agc_extend(literal);ret=(agc_word)(q+1u);}
 else if(entry==CM_RECORD_VARALARM){w[1]=w[0];w[01363]=q;ret=q;q=05771;}
 else if(entry==CM_RECORD_CURTAINS){w[01363]=q;ret=q;q=05762;w[1]=00217;}
 else{w[1]=w[0];ret=(agc_word)(q+1u);}
 w[2]=q;w[01364]=(agc_word)(w[6]|io->superbank);w[061]=q;w[057]=w[0164];w[060]=w[0165];
 first=agc_classify(w[0375]);a=agc_extend(agc_diminished_abs(w[0375]));
 if(first==AGC_PLUS_ZERO||first==AGC_NEGATIVE){if(first==AGC_PLUS_ZERO)lxch(w,0375);lamp=1;}
 else{second=agc_classify(w[0376]);a=agc_extend(agc_diminished_abs(w[0376]));
  if(second==AGC_PLUS_ZERO||second==AGC_NEGATIVE){if(second==AGC_PLUS_ZERO)lxch(w,0376);}
  else{agc_word masked=(agc_word)(w[0377]&037777u);a=agc_extend(agc_diminished_abs(masked));if(!masked)lxch(w,0377);else{a=agc_add_register(w[1],agc_extend(040000));w[0377]=agc_correct(a);if(agc_overflow(a)){w[0]=agc_overflow(a)>0?1u:0177776u;w[5]=05721;return EXEC_OK;}}}
 }
 if(lamp){a=agc_add_register(agc_extend((agc_word)((agc_negate(w[01036]))&040400u)),agc_extend(w[01036]));w[01036]=agc_correct(a);}
 w[061]=agc_correct(a);w[0]=agc_extend(q);io->interrupt_enabled=1;w[2]=(entry==CM_RECORD_VARALARM||entry==CM_RECORD_CURTAINS)?01364:05715;w[5]=ret;return EXEC_OK;
}
