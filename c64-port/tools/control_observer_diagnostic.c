/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "control_observer_vectors.h"
unsigned control_observer_check(void);
int main(void){volatile uint8_t *m=(volatile uint8_t *)0xc080;unsigned f;m[0]=0;f=control_observer_check();m[1]=(uint8_t)f;m[2]=(uint8_t)(f>>8);m[3]=CONTROL_OBSERVER_ROWS;m[4]=0;m[0]=0xa5;return f?1:0;}
