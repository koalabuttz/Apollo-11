/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_blanksub_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_blanksub_check(n,&image,&before));dsky_blanksub_input(&image,&io,1);image.words[01032]=0;before=image;old=io;if(dsky_blanksub(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;dsky_blanksub_input(&image,&io,0);image.words[01012]=2;before=image;old=io;if(dsky_blanksub(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;printf("LM+CM complete BLANKSUB:64 full source states,%u failures\n",errors);return errors!=0;}
