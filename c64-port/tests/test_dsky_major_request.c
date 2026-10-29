/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_major_request_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_major_request_route route=DSKY_MAJOR_REQUEST_V37;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_major_request_check(n,&image,&before));dsky_major_request_input(&image,&io,2);image.words[0136]=0;before=image;old=io;if(dsky_major_request(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MAJOR_REQUEST_V37)++errors;printf("LM+CM major-mode request:64 full source states,%u failures\n",errors);return errors!=0;}
