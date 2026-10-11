/* SPDX-License-Identifier: MIT; source Luminary099 idle Executive. */
#include "lm_idle_dispatch.h"
enum exec_status lm_idle_dispatch(apollo_overlay_state*image,p66_bias_io*io,
 agc_word*channel11,enum lm_idle_dispatch_entry entry,exec_transfer*next){
 static const agc_word pcs[3]={03214,03011,03225};
 uint16_t *w,flag,oldbb,debris,location,bank,outloc,outbank,outpriority,inpriority,outpush,inpush;
 uint8_t i,interpretive;
 if(!image||!io||!channel11||!next||(unsigned)entry>2u)return EXEC_OUT_OF_SCOPE;
 w=image->words;flag=w[067];oldbb=w[6];debris=agc_correct(w[1]);
 if(w[5]!=pcs[entry]||w[4]!=02000u||w[3]>03400u||(w[3]&0377u)||
  oldbb!=(uint16_t)(02000u|(w[3]>>8))||w[7]||(io->superbank&~0160u)||
  io->interrupt_enabled>1u||*channel11>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_IDLE_ADVAN&&flag==AGC_NEG_ZERO){
  location=w[01361];if(!location||location>07777u)return EXEC_OUT_OF_SCOPE;
  /* ADVAN's literal SELFRET/SELFBANK/SUPDXCHZ+1 does not alter core words. */
  w[0]=agc_extend(066102);w[1]=agc_extend(location);w[2]=2;w[3]=01000;
  w[4]=066000;w[6]=066002;w[5]=location;io->superbank=0100;
  next->address=(exec_address){location,066102};next->interpretive=0;return EXEC_IDLE;
 }
 if(entry==LM_IDLE_NUDIRECT){
  if(w[0]!=2||flag>AGC_MASK)return EXEC_OUT_OF_SCOPE;
  /* No NEWJOB check exists after the source CAF TWO. A newly posted pending
   * request must survive this already-selected NUDIRECT route unchanged. */
  flag=0;
 }else if(flag>84u||flag%12u)return EXEC_OUT_OF_SCOPE;
 location=w[0164u+flag];bank=w[0165u+flag];interpretive=(uint8_t)((location&AGC_SIGN)!=0);
 if((bank&~076167u)||(!interpretive&&(!location||location>07777u))||
  (interpretive&&agc_negate(location)>=07777u)||(!flag&&interpretive))return EXEC_OUT_OF_SCOPE;
 if(!flag&&(!w[0167]||w[0167]>=AGC_SIGN))return EXEC_OUT_OF_SCOPE;
 if(flag&&(!w[0167u+flag]||w[0167u+flag]>=AGC_SIGN))return EXEC_OUT_OF_SCOPE;
 *channel11=(agc_word)(*channel11|2u);
 if(!flag){
  w[0164]=*channel11;w[0165]=debris;w[0]=agc_extend(bank);w[1]=agc_extend(location);w[2]=2;
  w[6]=(uint16_t)(bank&076007u);w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);
  w[5]=location;io->superbank=(uint16_t)(bank&0160u);
  if(entry==LM_IDLE_NUCHANG2)io->interrupt_enabled=1;
  next->address=(exec_address){location,bank};next->interpretive=0;return EXEC_OK;
 }
 /* NUCHANG2's channel WOR and DXCH LOC precede CHANJOB+4. Saved outgoing
  * BANKSET already includes SUPER: no additional ROR or fabricated caller. */
 outloc=w[0164];outbank=w[0165];outpriority=w[0167];inpriority=w[0167u+flag];
 outpush=w[0166];inpush=w[0166u+flag];w[0164u+flag]=outloc;w[0165u+flag]=outbank;
 for(i=0;i<8u;++i){uint16_t old=w[0154u+i];w[0154u+i]=w[0154u+flag+i];w[0154u+flag+i]=old;}
 if(w[0121]&&w[0121]!=AGC_NEG_ZERO)outpush=agc_negate(outpush);
 w[0166u+flag]=outpush;w[0167u+flag]=outpriority;w[0167]=inpriority;
 w[0120]=(uint16_t)(inpriority&0777u);w[0121]=0;
 if(inpush&AGC_SIGN){inpush=agc_negate(inpush);w[0121]=1;}
 w[0166]=inpush;w[067]=0;w[0164]=0;w[0165]=debris;
 w[1]=oldbb;w[6]=(uint16_t)(bank&076007u);w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);
 io->superbank=(uint16_t)(bank&0160u);io->interrupt_enabled=1;
 if(interpretive){location=agc_add_word(agc_negate(location),1);w[0164]=location;w[0165]=w[6];
  w[0115]=(uint16_t)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06070;
 }else{w[0]=02772;w[5]=location;}
 next->address=(exec_address){location,bank};next->interpretive=interpretive;return EXEC_OK;
}
