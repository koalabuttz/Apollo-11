/* SPDX-License-Identifier: MIT */
#ifndef LM_WAITLIST_IRQ_DISPATCH_H
#define LM_WAITLIST_IRQ_DISPATCH_H
#include "lm_waitlist_dispatch.h"
/* Architectural T3 vector with source AllowInterrupt1/InIsr1. InIsr blocks
 * nested interrupts; literal vector/body contains no INHINT. Preserve the
 * source enable bit until the selected task performs its own operation. */
enum wl_status lm_waitlist_irq_dispatch(apollo_overlay_state *,p66_bias_io *,uint16_t,exec_address *);
#endif
