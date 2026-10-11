/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_extended_dispatch_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_extended_dispatch_check(n,&image,&before));dsky_extended_dispatch_input(&image,&io,32);image.words[0154]=60;before=image;old=io;if(dsky_extended_dispatch(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;dsky_extended_dispatch_input(&image,&io,32);io.superbank=0;before=image;old=io;if(dsky_extended_dispatch(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;printf("LM+CM extended verb fanout:64 full source states,%u failures\n",errors);return errors!=0;}
