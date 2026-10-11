/* SPDX-License-Identifier: MIT */
#ifndef CM_DYNDISP_CASES_H
#define CM_DYNDISP_CASES_H
#include "cm_dyndisp.h"
#define CM_DYNDISP_CASES 512u
#define CM_DYNDISP_CHECKPOINTS 27u
void cm_dyndisp_input(apollo_overlay_state *,cm_native_io *,uint16_t);
uint8_t cm_dyndisp_apply(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
