/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_mode_job_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0,mission,kind;uint8_t n;p66_bias_io io,old;enum dsky_mode_job_route route;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_mode_job_check(n,&image,&before));
 for(mission=0;mission<2u;++mission)for(kind=0;kind<8u;++kind){dsky_mode_job_input(&image,&io,(uint16_t)(mission*4096u));if(kind==0u)image.words[01011]=100;else if(kind==1u)image.words[4]=062000;else if(kind==2u)image.words[3]=1;else if(kind==3u)io.superbank=0;else if(kind==4u)io.interrupt_enabled=2;else if(kind==5u)++image.words[5];else if(kind==6u)image.words[7]=1;else image.words[6]=0;before=image;old=io;route=DSKY_MODE_JOB_CCSHOLE;
  if(mission==0u){if(dsky_mode_job(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MODE_JOB_CCSHOLE)++errors;}
  else{cm_native_io cm={0},cmold;cm.superbank=io.superbank;cm.interrupt_enabled=io.interrupt_enabled;cmold=cm;if(cm_mode_job(&image,&cm,&route)!=EXEC_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&cm,&cmold,sizeof cm)||route!=DSKY_MODE_JOB_CCSHOLE)++errors;}
 }
 printf("LM+CM DSPMMJB formatting:64 full source states,16 atomic guard states,%u failures\n",errors);return errors!=0;}
