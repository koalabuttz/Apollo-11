/* SPDX-License-Identifier: MIT; original Luminary099 INTERPRETER scheduler check. */
#include "lm_danzig_change.h"
enum exec_status lm_danzig_change(apollo_overlay_state*image,p66_bias_io*io,exec_transfer*next){uint16_t*w,offset,loc,bank;uint8_t interpretive;
 if(!image||!io||!next)return EXEC_OUT_OF_SCOPE;
 w=image->words;offset=w[067];
 if((w[5]!=06061u&&w[5]!=06063u)||w[7]||w[023]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||offset==0||offset>84u||offset%12u||w[0167]==0||w[0167]>=040000u||w[0167u+offset]==0||w[0167u+offset]>=040000u||w[0164]==0||w[0164]>07777u||(w[0165]&~076167u))return EXEC_OUT_OF_SCOPE;
 loc=w[0164u+offset];bank=w[0165u+offset];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);
 if((bank&~076167u)||(!interpretive&&(!loc||loc>07777u))||(interpretive&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;
 if(w[5]==06061u){w[6]=(uint16_t)(w[0165]&076007u);w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);}
 /* CCS EDOP+0 skips OPJUMP; CCS positive NEWJOB selects actual TCF CHANG2.
  * TCF retains Q. No numerical operation or instruction word is decoded. */
 w[0]=(uint16_t)(offset-1u);w[5]=05126;
 return lm_interpreter_change(image,io,next);
}
