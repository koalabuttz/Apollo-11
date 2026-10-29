/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_enter_zero_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;enum dsky_enter_zero_route route=DSKY_ZERO_MIXED;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_enter_zero_check(n,&image,&before));dsky_enter_zero_input(&image,&io,0);image.words[01002]=100;before=image;old=io;if(dsky_enter_zero(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ZERO_MIXED)++errors;dsky_enter_zero_input(&image,&io,0);io.superbank=0;before=image;old=io;if(dsky_enter_zero(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_ZERO_MIXED)++errors;printf("LM+CM ENTPAS0 noun admission:64 full source states,%u failures\n",errors);return errors!=0;}
