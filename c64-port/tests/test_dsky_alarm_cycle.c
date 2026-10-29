/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_alarm_cycle_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_alarm_cycle_route route=DSKY_ALARM_CYCLE_ENTER;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_alarm_cycle_check(n,&image,&before));dsky_alarm_cycle_input(&image,&io,1);image.words[0133]=0;before=image;old=io;if(dsky_alarm_cycle(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ALARM_CYCLE_ENTER)++errors;printf("LM+CM alarm-cycle bridges:64 full source states,%u failures\n",errors);return errors!=0;}
