/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_charin_reset_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_reset_io io,old;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_charin_reset_check(n,&image,&before));dsky_charin_reset_input(&image,&io,0);image.words[0154]=021;before=image;old=io;if(dsky_charin_reset(&image,&io)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io))++errors;printf("LM+CM CHARIN RESET:64 full source states,%u failures\n",errors);return errors!=0;}
