/* SPDX-License-Identifier: MIT */
#include "c64_dsky_display.h"
void c64_dsky_display_render(const dsky_display_state *s,uint8_t blink_visible){dsky_display_render(s,(volatile uint8_t *)0x0400,(volatile uint8_t *)0xd800,blink_visible);}
