/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_load_store_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;dsky_lamp_io io,old;enum dsky_load_store_route route;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_load_store_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<13u;++kind){dsky_load_store_input(&image,&io,(uint16_t)(mission*4096u));if(kind==0u)image.words[3]=1;else if(kind==1u)io.control.superbank=0;else if(kind==2u)io.restart_light=2;else if(kind==3u)image.words[4]=060000;else if(kind==4u)image.words[5]=03377;else if(kind==5u)io.control.interrupt_enabled=2;else if(kind==6u)image.words[6]=1;else if(kind==7u)image.words[7]=1;else if(kind==8u){image.words[5]=02641;image.words[0]=0100000;image.words[0145]=01200;}else if(kind==9u){image.words[5]=02641;image.words[0]=0;image.words[0145]=077777;}else if(kind==10u){image.words[5]=02641;image.words[0]=0;image.words[0145]=07;}else if(kind==11u){image.words[5]=02660;image.words[01003]=0;}else image.words[01002]=100;before=image;old=io;route=DSKY_LOAD_STORE_RECYCLE;
  if(mission==0u){if(dsky_load_store(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_LOAD_STORE_RECYCLE)++errors;}
  else{cm_native_io cm={0},cmold;uint16_t wrote=0137,restart=io.restart_light;cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cm.channel12=io.channel12;cm.channel13=io.channel13;cmold=cm;if(cm_load_store(&image,&cm,&wrote,&restart,&route)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||wrote!=0137u||restart!=io.restart_light||route!=DSKY_LOAD_STORE_RECYCLE)++errors;}
 }
 printf("LM+CM load caller stores:64 full source states,26 atomic guard states,%u failures\n",errors);return errors!=0;}
