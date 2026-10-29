/* SPDX-License-Identifier: MIT */
#include "dsky_test_noun_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t failures=0,n;mail[0]=0;for(n=DSKY_TEST_NOUN_COHORT*64u;n<(DSKY_TEST_NOUN_COHORT+1u)*64u;++n)failures=(uint16_t)(failures+dsky_test_noun_checkpoint(n,&s,&before));mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=64;mail[4]=DSKY_TEST_NOUN_COHORT;mail[0]=0xa5;return failures?1:0;}
