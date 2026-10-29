/* SPDX-License-Identifier: MIT */
#ifndef CM_FORMATTER_CASES_H
#define CM_FORMATTER_CASES_H
#include "cm_formatter.h"
#define CM_FORMATTER_CASES 12288u
#define CM_FORMATTER_TARGET_COUNT 64u
void cm_formatter_input(apollo_overlay_state *,cm_native_io *,uint16_t);
enum cm_formatter_status cm_formatter_run(apollo_overlay_state *,cm_native_io *,uint16_t);
#endif
