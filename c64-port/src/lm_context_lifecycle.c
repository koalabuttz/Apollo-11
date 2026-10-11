/* SPDX-License-Identifier: MIT; original Luminary099 Executive lifecycle. */
#include "lm_context_lifecycle.h"
enum exec_status lm_context_lifecycle(apollo_overlay_state *image,p66_bias_io *io,
 agc_word *channel11,enum lm_context_lifecycle_entry entry,exec_transfer *next){
 static const agc_word calls[7]={03122,03126,03132,03136,03142,03146,03152};
 uint16_t *w,oldbank,oldl,packed,highest,buf0,buf1,buf2,q,location=0,bank=0;
 uint16_t outloc,outbank,outpriority,inpriority,outpush,inpush,oldcorebank,offset;
 uint8_t i,winner=0,interpretive=0;
 if(!image||!io||!channel11||!next||(unsigned)entry>1u)return EXEC_OUT_OF_SCOPE;
 w=image->words;oldbank=w[6];oldl=w[1];q=w[2];
 if(w[5]!=(entry==LM_CONTEXT_PRIORITY?05146u:05133u)||w[7]||w[3]>03400u||
  (w[3]&0377u)||(w[4]&~076000u)||oldbank!=(uint16_t)(w[4]|(w[3]>>8))||
  (io->superbank&~0160u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||
  !w[0167]||w[0167]>=AGC_SIGN||agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;
 if(entry==LM_CONTEXT_PRIORITY){
  if(q<0100u||q>07777u)return EXEC_OUT_OF_SCOPE;
  packed=agc_add_word(agc_correct(w[0]),(uint16_t)(w[0167]&0777u));
  if(packed>=AGC_SIGN)return EXEC_OUT_OF_SCOPE;
  highest=packed;buf0=0;buf1=agc_negate(packed);
 }else{packed=w[0167];highest=0;buf0=w[0130];buf1=AGC_NEG_ZERO;}
 buf2=w[0132];
 for(i=1;i<8u;++i){uint16_t priority=w[0167u+12u*i];
  if(!priority)return EXEC_OUT_OF_SCOPE;
  if(priority<AGC_SIGN){buf2=(uint16_t)(priority-1u);
   if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03206;}
   else q=03201;}}
 if(winner){location=w[0164u+12u*winner];bank=w[0165u+12u*winner];
  interpretive=(uint8_t)((location&AGC_SIGN)!=0);
  if((bank&~076167u)||(!interpretive&&(!location||location>07777u))||
   (interpretive&&agc_negate(location)>=07777u))return EXEC_OUT_OF_SCOPE;}
 /* Compile the literal entry preparations and EJSCAN, retaining the source
  * CCS(priority)-1 rule. Raw priority1 is a zero candidate, never a winner. */
 if(entry==LM_CONTEXT_PRIORITY){w[063]=agc_correct(w[0]);w[0164]=w[2];
  w[0165]=oldbank;w[0167]=packed;oldcorebank=oldbank;outloc=w[2];outbank=oldbank;
  w[6]=02001;w[3]=0400;w[4]=02000;
 }else{outloc=agc_correct(w[0]);w[0164]=outloc;w[0167]=agc_negate(packed);
  oldcorebank=(uint16_t)((oldbank&7u)|io->superbank);
  w[0165]=oldcorebank;outbank=oldcorebank;w[4]=02000;w[6]=(uint16_t)(02000u|(oldbank&7u));}
 w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[2]=q;io->interrupt_enabled=1;
 if(!winner){
  if(entry==LM_CONTEXT_SLEEP||packed==0){w[067]=AGC_NEG_ZERO;*channel11=(agc_word)(*channel11&077775u);
   w[0]=agc_extend(*channel11);w[1]=agc_extend(agc_correct(oldl));w[5]=03214;
   next->address=(exec_address){0,0};next->interpretive=0;return EXEC_IDLE;}
  /* ENDPRCHG-1 clears NEWJOB and the unchanged basic DXCH/DTCB returns. */
  w[067]=0;w[0164]=0;w[0165]=agc_correct(oldl);w[0]=02772;w[1]=02001;
  w[5]=outloc;w[6]=oldbank;w[3]=(uint16_t)((oldbank&7u)*0400u);w[4]=(uint16_t)(oldbank&076000u);
  next->address=(exec_address){outloc,(uint16_t)(oldbank|io->superbank)};next->interpretive=0;
  return EXEC_OK;
 }
 /* CHANJOB-2 supplies the already saved outgoing LOC and BANKSET. Source
  * exchange has no native pointer, decoder, or implicit lifecycle call. */
 offset=(uint16_t)(12u*winner);outpriority=w[0167];inpriority=w[0167u+offset];outpush=w[0166];inpush=w[0166u+offset];
 w[0164u+offset]=outloc;w[0165u+offset]=(uint16_t)(outbank|io->superbank);
 for(i=0;i<8u;++i){uint16_t old=w[0154u+i];w[0154u+i]=w[0154u+offset+i];w[0154u+offset+i]=old;}
 if(w[0121]&&w[0121]!=AGC_NEG_ZERO)outpush=agc_negate(outpush);
 w[0166u+offset]=outpush;w[0167u+offset]=outpriority;w[0167]=inpriority;
 w[0120]=(uint16_t)(w[0167]&0777u);w[0121]=0;
 if(inpush&AGC_SIGN){inpush=agc_negate(inpush);w[0121]=1;}
 w[0166]=inpush;w[067]=0;w[0164]=0;w[0165]=oldcorebank;
 w[1]=w[6];w[6]=(uint16_t)(bank&076007u);w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);
 io->superbank=(uint16_t)(bank&0160u);
 if(interpretive){location=agc_add_word(agc_negate(location),1);w[0164]=location;w[0165]=w[6];
  w[0115]=(uint16_t)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06070;
 }else{w[0]=02772;w[5]=location;}
 next->address=(exec_address){location,bank};next->interpretive=interpretive;return EXEC_OK;
}
