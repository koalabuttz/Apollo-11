/* SPDX-License-Identifier: MIT */
#ifndef LM_WAKE_IMAGE_H
#define LM_WAKE_IMAGE_H
#include "p66_bias.h"
#include "executive.h"
/* Literal JOBWAKE through ENDFIND/Q+2. Includes a dormant core zero and
 * pending-request comparison. Does not execute awakened jobs or poll IRQs. */
enum exec_status lm_wake_image(apollo_overlay_state *,p66_bias_io *);
#endif
