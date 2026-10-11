/* SPDX-License-Identifier: MIT */
#ifndef INTERPRETED_SIGN_CASES_H
#define INTERPRETED_SIGN_CASES_H
#include "interpreted_sign.h"
#define INTERPRETED_SIGN_CASES UINT32_C(131072)
void interpreted_sign_input(apollo_overlay_state *,agc_word *,
 enum interpreted_sign_mission,uint32_t);
#endif
