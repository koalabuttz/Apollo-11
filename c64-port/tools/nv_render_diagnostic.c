/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "nv_render_check.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned f;mail[0]=0;f=nv_render_check();mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=16;mail[4]=NV_RENDER_SELECTED;mail[0]=0xa5;return f?1:0;}
