/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_alarm_recycle_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_recycle_route route=DSKY_RECYCLE_CCS_HOLE;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_alarm_recycle_check(n,&image,&before));dsky_alarm_recycle_input(&image,&io,0);image.words[01041]=agc_negate(100);before=image;old=io;if(dsky_alarm_recycle(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_RECYCLE_CCS_HOLE)++errors;printf("LM+CM continuous alarm recycle:64 full source states,%u failures\n",errors);return errors!=0;}
