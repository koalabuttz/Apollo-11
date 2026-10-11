/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned rtb_unit_check(void);
int main(void){volatile uint8_t *m=(volatile uint8_t *)0xc080;unsigned f;m[0]=0;f=rtb_unit_check();m[1]=(uint8_t)f;m[2]=(uint8_t)(f>>8);m[3]=32;m[4]=(uint8_t)(RTBU_MISSION*32+RTBU_CALLER*4+RTBU_PHASE);m[0]=0xa5;return f?1:0;}
