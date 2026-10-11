/* SPDX-License-Identifier: MIT */
#ifndef CM_NV85_CHECK_H
#define CM_NV85_CHECK_H
#include "cm_nv85_cases.h"
uint8_t cm_nv85_caller(apollo_overlay_state *,cm_native_io *,uint8_t);
uint8_t cm_nv85_checkpoint_input(uint8_t,apollo_overlay_state *,cm_native_io *);
uint16_t cm_nv85_checkpoint_check(uint8_t,apollo_overlay_state *,apollo_overlay_state *);
#endif
