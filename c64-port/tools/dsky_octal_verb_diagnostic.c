/* SPDX-License-Identifier: MIT */
#include "dsky_octal_verb_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t failures=0;uint8_t n;mail[0]=0;for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_octal_verb_checkpoint(n,&s,&before));mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=128;mail[4]=0;mail[0]=0xa5;return failures?1:0;}
