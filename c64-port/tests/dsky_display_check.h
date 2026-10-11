/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DISPLAY_CHECK_H
#define DSKY_DISPLAY_CHECK_H
#include "dsky_display.h"
/* Source-native mode66 formatting -> real channel10 relay -> screen result,
 * plus independent wiring/sign/blank/flash/annunciator cases. */
uint16_t dsky_display_check(volatile uint8_t *,volatile uint8_t *);
#endif
