/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_load_control_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_load_control_check(n,&image,&before));dsky_load_control_input(&image,&io,64);image.words[0167]=0;image.words[01042]=1;before=image;old=io;if(dsky_load_control(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;dsky_load_control_input(&image,&io,0);image.words[5]=02000;before=image;old=io;if(dsky_load_control(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;printf("LM+CM load response recall:64 full source states,%u failures\n",errors);return errors!=0;}
