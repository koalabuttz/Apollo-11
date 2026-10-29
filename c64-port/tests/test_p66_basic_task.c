/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_basic_task_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t errors=0;for(n=0;n<64;++n)errors=(uint16_t)(errors+p66_basic_task_check(n,&state,&before));printf("P66 BASIC RODTASK:64 complete2050word original source images, %u failures\n",errors);return errors!=0;}
