/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_enter_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_enter_route route=DSKY_ENTER_ALARM;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_enter_check(n,&image,&before));dsky_enter_input(&image,&io,0);image.words[0154]=021;before=image;old=io;if(dsky_enter(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ENTER_ALARM)++errors;dsky_enter_input(&image,&io,2048);image.words[4]=060000;before=image;old=io;if(dsky_enter(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ENTER_ALARM)++errors;printf("LM+CM ENTER pass selection:64 full source states,%u failures\n",errors);return errors!=0;}
