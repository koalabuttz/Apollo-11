/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_machine_cadr_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;p66_bias_io io,old;enum dsky_machine_route route=DSKY_MACHINE_CCS_HOLE;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_machine_cadr_check(n,&image,&before));dsky_machine_cadr_input(&image,&io,0);image.words[5]=02117;before=image;old=io;if(dsky_machine_cadr(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_MACHINE_CCS_HOLE)++errors;printf("LM+CM machine-address caller:64 full source states,%u failures\n",errors);return errors!=0;}
