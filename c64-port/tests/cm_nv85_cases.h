/* SPDX-License-Identifier: MIT */
#ifndef CM_NV85_CASES_H
#define CM_NV85_CASES_H
#include "cm_nv85.h"
#define CM_NV85_CASES 512u
#define CM_NV85_CHECKPOINTS 26u
void cm_nv85_input(apollo_overlay_state *,cm_native_io *,uint16_t);
uint8_t cm_nv85_apply(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
