/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_verb_dispatch_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;enum dsky_verb_route route=DSKY_VERB_EXTENDED_RELEASE;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_verb_dispatch_check(n,&image,&before));dsky_verb_dispatch_input(&image,&io,32);image.words[01001]=100;before=image;old=io;if(dsky_verb_dispatch(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_VERB_EXTENDED_RELEASE)++errors;dsky_verb_dispatch_input(&image,&io,32);io.superbank=0;before=image;old=io;if(dsky_verb_dispatch(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_VERB_EXTENDED_RELEASE)++errors;printf("LM+CM VERBFAN dispatch:64 full source states,%u failures\n",errors);return errors!=0;}
