/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_blank_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_blank_check(n,&image,&before));dsky_blank_input(&image,&io,1);image.words[01032]=0;before=image;old=io;if(dsky_blank(&image,&io,DSKY_BLNKSUB1)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;dsky_blank_input(&image,&io,2048);image.words[0]|=010;before=image;old=io;if(dsky_blank(&image,&io,DSKY_BLANK_RETURN)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;printf("LM+CM blanking / bank return:64 full source states,%u failures\n",errors);return errors!=0;}
