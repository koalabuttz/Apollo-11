/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_consumer_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t n,fail=0;mail[0]=0;for(n=0;n<((DSKY_MONITOR_CONSUMER_ONLY_STAGE==23u||DSKY_MONITOR_CONSUMER_ONLY_STAGE==13u)?8u:16u);++n)fail=(uint16_t)(fail+dsky_monitor_consumer_checkpoint(DSKY_MONITOR_CONSUMER_ONLY_STAGE,n,&s,&before));mail[1]=(uint8_t)fail;mail[2]=(uint8_t)(fail>>8);mail[3]=(DSKY_MONITOR_CONSUMER_ONLY_STAGE==23u||DSKY_MONITOR_CONSUMER_ONLY_STAGE==13u)?8:16;mail[4]=DSKY_MONITOR_CONSUMER_ONLY_STAGE;mail[0]=0xa5;return fail?1:0;}
