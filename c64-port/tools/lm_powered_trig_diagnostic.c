/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned lm_powered_trig_check(void);
int main(void){volatile uint8_t *m=(volatile uint8_t *)0xc080;unsigned f;m[0]=0;f=lm_powered_trig_check();m[1]=(uint8_t)f;m[2]=(uint8_t)(f>>8);m[3]=32;m[4]=LM_POWERED_TRIG_SELECTED;m[0]=0xa5;return f?1:0;}
