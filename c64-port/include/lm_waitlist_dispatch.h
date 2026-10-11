/* SPDX-License-Identifier: MIT */
#ifndef LM_WAITLIST_DISPATCH_H
#define LM_WAITLIST_DISPATCH_H
#include "lm_taskover_resume.h"
#include "waitlist.h"
/* T3 vector4014 or TASKOVER5261/RUPTAGN+1 through the selected basic task
 * entry, preserving exact saved IRQ registers and all list/CPU debris.
 * Caller supplies already captured ZRUPT/BRUPT and in_interrupt1; no source
 * instruction is executed from BRUPT. Selected task remains a compiled entry. */
enum wl_status lm_waitlist_dispatch(apollo_overlay_state *,p66_bias_io *,uint16_t in_interrupt,exec_address *);
#endif
