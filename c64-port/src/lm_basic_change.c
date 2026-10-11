/* SPDX-License-Identifier: MIT; original Luminary099 CHANG1/CHANJOB. */
#include "lm_basic_change.h"
enum exec_status lm_basic_change(apollo_overlay_state *image,p66_bias_io *io,enum lm_basic_change_entry entry,exec_transfer *next){uint16_t *w,offset,loc,bank,out_priority,out_push,in_push,old_core_bank,outloc,outbank;uint8_t i,interpretive;
 if(!image||!io||!next||(unsigned)entry>1u)return EXEC_OUT_OF_SCOPE;
 w=image->words;offset=w[067];
 if(w[5]!=(entry==LM_CHANG1_ENTRY?05122u:02706u)||w[7]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||offset==0||offset>84u||offset%12u||w[0167]==0||w[0167]>=040000u||w[0167u+offset]==0||w[0167u+offset]>=040000u)return EXEC_OUT_OF_SCOPE;
 if(entry==LM_CHANG1_ENTRY){outloc=w[2];outbank=w[6];}
 else{if(w[6]!=02001u||agc_overflow(w[0])||agc_overflow(w[1]))return EXEC_OUT_OF_SCOPE;outloc=agc_correct(w[1]);outbank=agc_correct(w[0]);}
 if(!outloc||outloc>07777u||(outbank&~076007u))return EXEC_OUT_OF_SCOPE;
 loc=w[0164u+offset];bank=w[0165u+offset];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);
 if((bank&~076167u)||(!interpretive&&(!loc||loc>07777u))||(interpretive&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;
 old_core_bank=w[0165];out_priority=w[0167];out_push=w[0166];w[0164u+offset]=outloc;w[0165u+offset]=(uint16_t)(outbank|io->superbank);
 for(i=0;i<8u;++i){uint16_t old=w[0154u+i];w[0154u+i]=w[0154u+offset+i];w[0154u+offset+i]=old;}
 if(w[0121]&&w[0121]!=AGC_NEG_ZERO)out_push=agc_negate(out_push);
 in_push=w[0166u+offset];w[0166u+offset]=out_push;w[0167]=w[0167u+offset];w[0167u+offset]=out_priority;w[0120]=(uint16_t)(w[0167]&0777u);w[0121]=0;
 if(in_push&AGC_SIGN){in_push=agc_negate(in_push);w[0121]=1;}w[0166]=in_push;w[067]=0;w[0164]=0;w[0165]=old_core_bank;
 if(entry==LM_CHANG1_ENTRY)w[2]=w[1];
 w[6]=(uint16_t)(bank&076007u);w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);io->superbank=(uint16_t)(bank&0160u);io->interrupt_enabled=1;w[1]=02001;
 if(interpretive){loc=agc_add_word(agc_negate(loc),1);w[0164]=loc;w[0165]=w[6];w[0115]=(uint16_t)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06070;}
 else{w[0]=02772;w[5]=loc;}
 next->address.location=loc;next->address.bank=bank;next->interpretive=interpretive;return EXEC_OK;
}
