/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_response_recall_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;dsky_lamp_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_response_recall_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<8u;++kind){dsky_response_recall_input(&image,&io,(uint16_t)(mission*4096u+1u));if(kind==0u)image.words[3]=1;else if(kind==1u)io.control.superbank=0;else if(kind==2u)image.words[0167]=0;else if(kind==3u)image.words[0203]=0;else if(kind==4u)image.words[5]=03377;else if(kind==5u)io.restart_light=2;else if(kind==6u)image.words[067]=1;else image.words[7]=1;before=image;old=io;
  if(mission==0u){if(dsky_response_recall(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;}
  else{cm_native_io cm={0},cmold;uint16_t wrote=0137,restart=io.restart_light;cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;cmold=cm;if(cm_response_recall(&image,&cm,&wrote,&restart)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||wrote!=0137u||restart!=io.restart_light)++errors;}
 }
 printf("LM+CM response recall:64 full source states,16 atomic guard states,%u failures\n",errors);return errors!=0;}
