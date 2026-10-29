/* SPDX-License-Identifier: MIT */
#include "cm_init_display_mixed.h"
#include "cm_init_display.h"
enum exec_status cm_init_display_mixed_stage(apollo_overlay_state *s,cm_native_io *io,uint8_t phase,exec_transfer *next){agc_word *w;exec_address address;enum exec_status status;
 if(!s||!io||!next||phase>2u)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(phase==1u)return cm_priority_complete(s,io,next);
 if(w[3]>03400u||(w[3]&0377u)||w[4]!=020000u||w[6]!=(agc_word)(020000u|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(phase==0u){if(w[5]!=03261u||w[0366]>AGC_MASK||!((w[0366]&037000u))||w[01071]>AGC_MASK)return EXEC_OUT_OF_SCOPE;
  w[3]=(agc_word)(w[01071]&03400u);w[6]=(agc_word)(w[4]|(w[3]>>8));w[1]=agc_extend(agc_correct(w[1]));io->superbank=(agc_word)(w[0366]&0160u);w[0]=(agc_word)(w[0366]&037000u);w[2]=03267;w[5]=05210;next->interpretive=0;next->address=(exec_address){w[5],(agc_word)(w[6]|io->superbank)};return EXEC_OK;}
 if(w[5]!=03267u||w[0374]>AGC_MASK||(w[0374]&01777u)<4u)return EXEC_OUT_OF_SCOPE;
 status=cm_init_display_return(s,io,&address);if(status==EXEC_OK){next->interpretive=0;next->address=address;}return status;
}
