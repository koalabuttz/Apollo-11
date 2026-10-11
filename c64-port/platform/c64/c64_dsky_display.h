/* SPDX-License-Identifier: MIT */
#ifndef C64_DSKY_DISPLAY_H
#define C64_DSKY_DISPLAY_H
#include "dsky_display.h"
/* Native VIC screen0400 and colorD800 in standard KERNAL text mode; caller chooses when hardware display
 * owns screen. Channel output can be serviced after releasing source image. */
void c64_dsky_display_render(const dsky_display_state *,uint8_t blink_visible);
#endif
