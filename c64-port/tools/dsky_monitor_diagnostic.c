/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_check.h"
static apollo_overlay_state s,before;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint16_t failures=0,n;mail[0]=0;for(n=0;n<32u;++n)failures=(uint16_t)(failures+dsky_monitor_checkpoint(n,&s,&before));
#if DSKY_MONITOR_ONLY_STAGE==0
 {static const uint16_t pc[10]={03237,03267,03270,03305,03306,03311,03312,03320,03347,03350};static const agc_word data[10]={060000,03275,062101,03275,062101,03321,062101,0144,075377,04124};unsigned m,i;for(m=0;m<2u;++m)for(i=0;i<10u;++i){agc_word out=05555,expected=i==9u&&m==1u?04140u:data[i];if(!dsky_monitor_literal((enum dsky_nv_mission)m,(uint16_t)(041u*02000u+pc[i]-02000u),&out)||out!=expected)++failures;}}
#endif
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=32;mail[4]=DSKY_MONITOR_ONLY_STAGE;mail[0]=0xa5;return failures?1:0;}
