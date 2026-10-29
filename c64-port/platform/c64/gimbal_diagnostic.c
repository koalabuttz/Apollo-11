/* SPDX-License-Identifier: MIT */
#include "gimbal_cases.h"
#include "gimbal_vectors.h"
static apollo_overlay_state state,before;
int main(void){volatile unsigned char *mailbox=(volatile unsigned char *)0xc000;
 gimbal_io io,saved;uint16_t row,i,k,position,expected,n,checked=0,base,out,mission;
 mailbox[0]=0xff;
 for(row=0;row<GIMBAL_TARGET_ROWS;++row){n=gimbal_target_ids[row];gimbal_prepare(&state,&io,n);before=state;saved=io;base=state.words[0120];
 if(!gimbal_entry(&state,&io,gimbal_mission(n)))goto failed;k=0;position=gimbal_changed[0];
 for(i=0;i<2048u;++i){expected=before.words[i];if(i==position){expected=gimbal_expected[row][k++];
 if(k<16u)position=gimbal_changed[k];else if(k==16u)position=(uint16_t)(base+046u);else if(k==17u)position=(uint16_t)(base+052u);else position=0xffffu;}
 if(state.words[i]!=expected)goto failed;}
 if(io.superbank!=saved.superbank || io.interrupt_enabled!=gimbal_expected[row][18])goto failed;++checked;}
 for(mission=0;mission<2u;++mission)for(i=0;i<(mission==0?10u:30u);++i){
 if(!gimbal_fixed_read((enum apollo_fixed_mission)mission,(uint16_t)((mission==0?18937u:18936u)+i),&out) || out!=gimbal_constant_expected[mission][i])goto failed;}
 mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xa5;for(;;){}
 failed:mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xee;for(;;){}
}
