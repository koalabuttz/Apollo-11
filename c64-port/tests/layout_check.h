/* SPDX-License-Identifier: MIT */
#ifndef LAYOUT_CHECK_H
#define LAYOUT_CHECK_H
#include "layout.h"
#define LAYOUT_CHECK_BINDINGS 4232u
bool layout_check(apollo_overlay_state *, volatile uint16_t *output);
#endif
