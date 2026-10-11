/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lm_radar_leadin_cases.h"
int main(void){unsigned n;apollo_overlay_state m,before;p66_bias_io io,old;
 for(n=0;n<17u;++n){lm_radar_input(&m,&io,0);
  switch(n){case 0:m.words[5]=02000;break;case 1:m.words[7]=1;break;case 2:m.words[3]=02400;break;case 3:m.words[4]=054000;break;case 4:m.words[6]=052005;break;case 5:io.superbank=1;break;case 6:io.interrupt_enabled=2;break;case 7:m.words[2]=0100000;break;case 8:m.words[02354]=1;break;case 9:m.words[02352]=1;break;case 10:m.words[02353]=1;break;case 11:m.words[02351]=6;break;case 12:m.words[026]=040000;break;case 13:m.words[067]=11;break;case 14:m.words[0167]=0;break;case 15:m.words[0400]=0401;break;default:m.words[067]=0;break;}
  before=m;old=io;assert(lm_radar_leadin(&m,&io)==LM_RADAR_REJECT);assert(memcmp(&m,&before,sizeof m)==0);assert(memcmp(&io,&old,sizeof io)==0);
 }
 lm_radar_input(&m,&io,0);before=m;old=io;assert(lm_radar_leadin(0,&io)==LM_RADAR_REJECT);assert(lm_radar_leadin(&m,0)==LM_RADAR_REJECT);assert(memcmp(&m,&before,sizeof m)==0);assert(memcmp(&io,&old,sizeof io)==0);
 puts("PASS:17 invalid source/domain states and2 null interfaces preserve full image and I/O");return 0;
}
