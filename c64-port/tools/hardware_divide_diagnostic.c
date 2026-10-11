/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "hardware_divide_vectors.h"
unsigned hardware_divide_check(void);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned f;mail[0]=0;f=hardware_divide_check();mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=(uint8_t)HARDWARE_DIVIDE_ROWS;mail[4]=(uint8_t)(HARDWARE_DIVIDE_ROWS>>8);mail[0]=0xa5;return f?1:0;}
