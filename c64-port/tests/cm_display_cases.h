/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPLAY_CASES_H
#define CM_DISPLAY_CASES_H
#include "cm_init_display.h"
#define CM_DISPLAY_CASES 12288u
#define CM_DISPLAY_TARGET_COUNT 64u
void cm_display_input(apollo_overlay_state *,cm_native_io *,uint16_t);
enum exec_status cm_display_run(apollo_overlay_state *,cm_native_io *,uint16_t,exec_address *);
#endif
