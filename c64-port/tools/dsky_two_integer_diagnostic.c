/* SPDX-License-Identifier: MIT */
#include "dsky_two_integer_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t failures=0,n;mail[0]=0;for(n=0;n<32u;++n)failures=(uint16_t)(failures+dsky_two_integer_checkpoint(n,&s,&before));mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=32;mail[4]=DSKY_TWO_INTEGER_ONLY_STAGE;mail[0]=0xa5;return failures?1:0;}
