/* SPDX-License-Identifier: MIT */
#ifndef C64_DSKY_KEYBOARD_H
#define C64_DSKY_KEYBOARD_H
#include <stdint.h>
enum c64_dsky_key_status {C64_DSKY_KEY_EMPTY=0,C64_DSKY_KEY_READY=1,C64_DSKY_KEY_UNMAPPED=2};
/* Actual KERNAL GETIN input, foreground safe point. Raw PETSCII is retained
 * for unmapped input; channel15 is unchanged unless a recognized key arrives.
 * Hardware input does not mutate flight state or invoke a job in an IRQ. */
enum c64_dsky_key_status c64_dsky_keyboard_poll(uint16_t *channel15,uint8_t *raw_character);
#endif
