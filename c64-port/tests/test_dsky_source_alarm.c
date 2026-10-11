/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_source_alarm_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_alarm_route route=DSKY_ALARM_ABORT;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_source_alarm_check(n,&image,&before));dsky_source_alarm_input(&image,&io,0);image.words[5]=02000;before=image;old=io;if(dsky_source_alarm(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ALARM_ABORT)++errors;dsky_source_alarm_input(&image,&io,5);image.words[2]=077777;before=image;old=io;if(dsky_source_alarm(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ALARM_ABORT)++errors;printf("LM+CM source alarms/lamps:64 full source states,%u failures\n",errors);return errors!=0;}
