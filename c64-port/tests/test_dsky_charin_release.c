/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_charin_release_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_release_route route=DSKY_RELEASE_PINBRANCH;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_charin_release_check(n,&image,&before));dsky_charin_release_input(&image,&io,0);image.words[0154]=021;before=image;old=io;if(dsky_charin_release(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_RELEASE_PINBRANCH)++errors;dsky_charin_release_input(&image,&io,65);image.words[0167u+12u]=0;before=image;old=io;if(dsky_charin_release(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_RELEASE_PINBRANCH)++errors;printf("LM+CM CHARIN KEYREL:64 full source states,%u failures\n",errors);return errors!=0;}
