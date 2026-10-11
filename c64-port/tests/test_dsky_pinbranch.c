/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_pinbranch_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;enum dsky_pinbranch_route route=DSKY_PINBRANCH_MARK;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_pinbranch_check(n,&image,&before));dsky_pinbranch_input(&image,&io,0);image.words[5]=02000;before=image;old=io;if(dsky_pinbranch(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_PINBRANCH_MARK)++errors;dsky_pinbranch_input(&image,&io,4096);image.words[6]^=1;before=image;old=io;if(dsky_pinbranch_resume(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_PINBRANCH_MARK)++errors;printf("LM+CM PINBRNCH / display continuation:64 full source states,%u failures\n",errors);return errors!=0;}
