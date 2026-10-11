/* SPDX-License-Identifier: MIT */
#ifndef P66_RODTASK_IMAGE_H
#define P66_RODTASK_IMAGE_H
#include "p66_startup.h"
#include "p66_bias.h"
/* Canonical RODTASK3263 through actual TASKOVER5261/BAILOUT5716; no task
 * reschedule, source IRQ restoration, idle dispatch or native queue poll. */
enum exec_status p66_rodtask_image(apollo_overlay_state *,p66_bias_io *);
#endif
