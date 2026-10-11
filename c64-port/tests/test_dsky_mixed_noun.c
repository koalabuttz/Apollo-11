/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_mixed_noun_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;enum dsky_mixed_route route=DSKY_MIXED_ALARM;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_mixed_noun_check(n,&image,&before));dsky_mixed_noun_input(&image,&io,32);image.words[0153]=077777;before=image;old=io;if(dsky_mixed_noun(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MIXED_ALARM)++errors;dsky_mixed_noun_input(&image,&io,32);io.superbank=0;before=image;old=io;if(dsky_mixed_noun(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MIXED_ALARM)++errors;printf("LM+CM mixed noun exchange:64 full source states,%u failures\n",errors);return errors!=0;}
