/* SPDX-License-Identifier: MIT */
/* The LM flight check's RESTARTS module (ABI 2 task slot): the
 * lm-exec-services programs' second routine table, which the kernel's
 * trampoline calls while the module stays in the task slot. Value: the
 * table's address. */
#include "lm_restart_routines.h"
#include "overlay_v2.h"
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){(void)selector;
 f->value=(uint16_t)(uintptr_t)&lm_restart_routines;return APOLLO_OV2_DONE;}
