/* SPDX-License-Identifier: MIT */
#ifndef CM_DYNDISP_CHECK_H
#define CM_DYNDISP_CHECK_H
#include "cm_dyndisp_cases.h"
uint8_t cm_dyndisp_checkpoint_input(uint8_t,apollo_overlay_state *,cm_native_io *);
uint16_t cm_dyndisp_checkpoint_check(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
