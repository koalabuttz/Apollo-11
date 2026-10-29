/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_address_request_check.h"
static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;dsky_charin_io io,old;enum dsky_request_route route=DSKY_REQUEST_CCS_HOLE;for(n=0;n<64;++n)errors=(uint16_t)(errors+dsky_address_request_check(n,&image,&before));dsky_address_request_input(&image,&io,2);image.words[01002]=100;before=image;old=io;if(dsky_address_request(&image,&io,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||route!=DSKY_REQUEST_CCS_HOLE)++errors;printf("LM+CM address request:64 full source states,%u failures\n",errors);return errors!=0;}
