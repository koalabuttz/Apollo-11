/* SPDX-License-Identifier: MIT */
#include "lm_torque_cases.h"
#include "lm_torque_vectors.h"
static apollo_overlay_state state,before;
int main(void){volatile unsigned char *mailbox=(volatile unsigned char *)0xc000;
 lm_torque_io io,saved;uint16_t row,bank,i,k,expected,position,checked=0;
 mailbox[0]=0xff;
 for(row=0;row<LM_TORQUE_TARGET_ROWS;++row)for(bank=0;bank<8u;++bank){
 lm_torque_prepare(&state,&io,(uint16_t)(lm_torque_target_ids[row]+bank));before=state;saved=io;
 if(!lm_torque(&state,&io))goto failed;
 k=0;position=lm_torque_changed[0];
 for(i=0;i<APOLLO_SHARED_WORDS;++i){expected=before.words[i];
  if(i==position){expected=lm_torque_expected[row][k++];
   position=k<LM_TORQUE_CHANGED_WORDS ? lm_torque_changed[k]:0xffffu;
   if(k>=4u && k<LM_TORQUE_CHANGED_WORDS)position=(uint16_t)(position+bank*0400u);}
  if(state.words[i]!=expected)goto failed;}
 if(io.superbank!=saved.superbank || io.interrupt_enabled!=saved.interrupt_enabled)goto failed;
 ++checked;}
 mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xa5;
 for(;;){}failed:mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xee;for(;;){}
}
