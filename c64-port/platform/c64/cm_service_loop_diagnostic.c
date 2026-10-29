/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "cm_service_loop_check.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned fail;mail[0]=0;fail=cm_service_loop_check();mail[1]=(uint8_t)fail;mail[2]=(uint8_t)(fail>>8);mail[3]=64;mail[4]=0;mail[0]=0xa5;for(;;){}return 0;}
