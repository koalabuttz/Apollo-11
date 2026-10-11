/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "guidance_control_observer.h"
int main(void){unsigned w[11],i;while(scanf("%u",&w[0])==1){p66_norm_scratch n;agc_dp a,b,c;for(i=1;i<11u;++i)if(scanf("%u",&w[i])!=1)return 2;a.high=(agc_word)w[2];a.low=(agc_word)w[3];b.high=(agc_word)w[4];b.low=(agc_word)w[5];c.high=(agc_word)w[6];c.low=(agc_word)w[7];n.addrwd=(agc_word)w[8];printf("%u\n",p66_acceleration_return_q((uint8_t)w[0],(agc_word)w[1],a,b,c,&n,(agc_word)w[9]));}return 0;}
