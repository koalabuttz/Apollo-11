/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_operator_schedule_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;dsky_charin_io io,old;enum dsky_operator_schedule_route route;exec_address pair;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_operator_schedule_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<4u;++kind){pair=dsky_operator_schedule_input(&image,&io,(uint16_t)(mission*4096u+4u));if(kind==0u)++pair.location;else if(kind==1u)image.words[01045]=0;else if(kind==2u){image.words[5]=03512;image.words[026]=077777;}else{image.words[01043]=02546;image.words[0167]=0;}before=image;old=io;route=DSKY_SCHEDULE_WAIT_BAD;
  if(mission==0u){if(dsky_operator_schedule(&image,&io,pair,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_SCHEDULE_WAIT_BAD)++errors;}
  else{cm_native_io cm={0},cmold;uint16_t wrote=0137;cm.superbank=io.control.superbank;cm.interrupt_enabled=io.control.interrupt_enabled;cm.channel11=io.channel11;cmold=cm;if(cm_operator_schedule(&image,&cm,&wrote,pair,&route)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||wrote!=0137u||route!=DSKY_SCHEDULE_WAIT_BAD)++errors;}
 }
 printf("LM+CM complete operator scheduler:64 full source states,8 atomic guard states,%u failures\n",errors);return errors!=0;}
