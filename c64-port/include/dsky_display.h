/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DISPLAY_H
#define DSKY_DISPLAY_H
#include <stdint.h>
/* Peripheral relay latches only, no duplicate native guidance state. */
typedef struct {uint16_t relay[12],valid,channel11;uint8_t flash_phase;} dsky_display_state;
void dsky_display_reset(dsky_display_state *);
/* Physical flashing:128centisecondperiod,32off/96on. App passes elapsed
 * native clock time; helper never reads/emulates AGC clocks or instructions. */
void dsky_display_advance(dsky_display_state *,uint16_t elapsed_centiseconds);
uint8_t dsky_display_flash_visible(const dsky_display_state *);
/* Channel10 selector0 removes relay drive but retains electromechanical
 * digit latches. Selectors1..11 numeric/sign rows,12 annunciator latch.
 * Channel11 direct lights and hardware verb/noun flashing. Unknown channel
 * or selector is ignored. All15-bit output words are ordinary hardwaredata. */
void dsky_display_output(dsky_display_state *,uint8_t channel,uint16_t word);
/* Render a 40x25 C64 screen/color RAM buffer. blink_visible is the native
 * hardware clock phase for the flashed verb/noun (1=visible). Decimal/octal
 * formatting already belongs to source-native Pinball, not this adapter. */
void dsky_display_render(const dsky_display_state *,volatile uint8_t *screen,
                         volatile uint8_t *color,uint8_t blink_visible);
#endif
