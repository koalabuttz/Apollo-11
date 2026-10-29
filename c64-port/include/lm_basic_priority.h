/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_PRIORITY_H
#define LM_BASIC_PRIORITY_H
#include "executive.h"
#include "p66_bias.h"
/* Actual LM PRIOCHNG5146 through basic DTCB dispatch. Canonical ordinary
 * erasable words, A upper priority01000..037000, Qordinary100..7777,
 * active core0, candidate priorities positive/dormant/-0; selected context
 * must be supported basic. Exact raw A/L/Q, scan BUF and SUPER/IRQ effects.
 * Returns actual selected pair in next; unsupported image/io/next unchanged.
 * No interpreter decoder or hidden CPU state; caller owns exclusion. */
enum exec_status lm_basic_priority_change(apollo_overlay_state *,p66_bias_io *,exec_address *next);
#endif
