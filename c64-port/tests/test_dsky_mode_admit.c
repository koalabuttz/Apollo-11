/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_mode_admit_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;p66_bias_io io,old;enum dsky_mode_admit_route route;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_mode_admit_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<8u;++kind){dsky_mode_admit_input(&image,&io,(uint16_t)(mission*4096u));if(kind==0u)image.words[2]=1;else if(kind==1u)image.words[4]=062000;else if(kind==2u)image.words[0167]=077777;else if(kind==3u)image.words[0203]=0;else if(kind==4u)image.words[0400]=1;else if(kind==5u)image.words[067]=1;else if(kind==6u)io.superbank=1;else io.interrupt_enabled=2;before=image;old=io;route=DSKY_MODE_ADMIT_ALARM;
  if(mission==0u){if(dsky_mode_admit(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MODE_ADMIT_ALARM)++errors;}
  else{cm_native_io cm={0},cmold;cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;cmold=cm;if(cm_mode_admit(&image,&cm,&route)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||route!=DSKY_MODE_ADMIT_ALARM)++errors;}
 }
 printf("LM+CM DSPMM admission:64 full source states,16 atomic guard states,%u failures\n",errors);return errors!=0;}
