/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned interpreted_square_check(void);
int main(void){volatile uint8_t *m=(volatile uint8_t *)0xc080;unsigned f;m[0]=0;f=interpreted_square_check();m[1]=(uint8_t)f;m[2]=(uint8_t)(f>>8);m[3]=64;m[4]=INTERPRETED_SQUARE_SELECTED;m[0]=0xa5;return f?1:0;}
