/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_lamp_lifecycle_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;dsky_lamp_io io,old;enum dsky_lamp_lifecycle_route route;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_lamp_lifecycle_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<8u;++kind){dsky_lamp_lifecycle_input(&image,&io,(uint16_t)(mission*4096u+3u));if(kind==0u)image.words[3]=1;else if(kind==1u)io.control.superbank=0;else if(kind==2u)image.words[0167]=0;else if(kind==3u)image.words[0203]=0;else if(kind==4u)image.words[5]=03614;else if(kind==5u)io.restart_light=2;else if(kind==6u)image.words[067]=1;else image.words[7]=1;before=image;old=io;route=DSKY_LAMP_LIFECYCLE_BLANK;
  if(mission==0u){if(dsky_lamp_lifecycle(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_LAMP_LIFECYCLE_BLANK)++errors;}
  else{cm_native_io cm={0},cmold;uint16_t wrote=0137,restart=io.restart_light;cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;cmold=cm;if(cm_lamp_lifecycle(&image,&cm,&wrote,&restart,&route)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||wrote!=0137u||restart!=io.restart_light||route!=DSKY_LAMP_LIFECYCLE_BLANK)++errors;}
 }
 printf("LM+CM lamp lifecycle:64 full source states,16 atomic guard states,%u failures\n",errors);return errors!=0;}
