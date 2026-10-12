/* SPDX-License-Identifier: MIT */
#ifndef INTERP_BODY_RUN_H
#define INTERP_BODY_RUN_H
#include "overlay_runtime.h"
/* Runs one interpreter body differential record on the image; returns the
 * index of the stop address reached (4: none, 5: unknown body). */
unsigned interp_body_run(apollo_overlay_state *,const uint16_t in[16]);
#endif
