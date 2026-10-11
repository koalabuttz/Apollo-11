/* SPDX-License-Identifier: MIT */
#ifndef P66_NATIVE_CHECK_H
#define P66_NATIVE_CHECK_H
#include "p66_native_cases.h"
unsigned pn_check(apollo_overlay_state *,executive *,exec_native *,velocity_case *,const exec_exclusion *,uint16_t (*)(void *,apollo_overlay_state *,uint16_t),void *);
#endif
