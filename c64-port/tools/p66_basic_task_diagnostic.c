/* SPDX-License-Identifier: MIT */
#include "p66_basic_task_check.h"
static apollo_overlay_state state,before;volatile unsigned char p66_basic_task_mailbox[5];
int main(void){uint8_t n;uint16_t errors=0;for(n=0;n<64;++n)errors=(uint16_t)(errors+p66_basic_task_check(n,&state,&before));p66_basic_task_mailbox[1]=(unsigned char)errors;p66_basic_task_mailbox[2]=(unsigned char)(errors>>8);p66_basic_task_mailbox[3]=64;p66_basic_task_mailbox[4]=2;p66_basic_task_mailbox[0]=0xa5;for(;;){}}
