/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_LIFECYCLE_CASES_H
#define CM_ABORT_LIFECYCLE_CASES_H
#include "cm_abort_lifecycle.h"
#define CM_ABORT_LIFECYCLE_CASES 1536u
agc_word cm_abort_lifecycle_input(apollo_overlay_state *,cm_native_io *,uint32_t,uint8_t *);
uint8_t cm_abort_lifecycle_group(uint32_t,uint8_t);
uint8_t cm_abort_lifecycle_count(uint32_t);
#endif
