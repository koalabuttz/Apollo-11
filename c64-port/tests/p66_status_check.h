/* SPDX-License-Identifier: MIT */
#ifndef P66_STATUS_CHECK_H
#define P66_STATUS_CHECK_H
#include "p66_native.h"
uint16_t p66_status_check(apollo_overlay_state *,executive *,exec_native *,
    const exec_exclusion *);
#endif
