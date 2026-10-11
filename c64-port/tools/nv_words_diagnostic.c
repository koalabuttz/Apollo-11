/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "nv_words_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t n;uint16_t failures=0;mail[0]=0;for(n=0;n<64u;++n)failures=(uint16_t)(failures+nv_words_check(n,&s,&before));mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=0;mail[0]=0xa5;return failures?1:0;}
