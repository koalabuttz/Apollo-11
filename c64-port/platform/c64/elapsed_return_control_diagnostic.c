/* SPDX-License-Identifier: MIT */
#include "p66_elapsed_return_control.h"
#include "elapsed_return_control_fixture.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;
 apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)0x7000;
 uint16_t failures=0,i,n;mail[0]=0;
 for(n=0;n<64u;++n){agc_dp before,pip;
  for(i=0;i<2048u;++i)image->words[i]=eq_initial[i];
  before.high=eq_target[n][0];before.low=eq_target[n][1];pip.high=eq_target[n][2];pip.low=eq_target[n][3];
  if(p66_elapsed_return_control(image,eq_target[n][4],before,pip)||image->words[2]!=eq_target[n][5])++failures;
  for(i=0;i<2048u;++i)if(i!=2u&&image->words[i]!=eq_initial[i])++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[5]=2;mail[0]=0xa5;
 return failures!=0;
}
