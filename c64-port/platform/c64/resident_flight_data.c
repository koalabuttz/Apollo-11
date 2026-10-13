/* SPDX-License-Identifier: MIT */
/* The flight check's data module (ABI 2 task slot, which the check
 * otherwise leaves free): the C64 rows and their frozen yaAGC digests and,
 * for LM, the RESTARTS routines the lm-exec-services programs chain into,
 * which the kernel's trampoline calls while the module stays loaded.
 * Value: the address of an rk_check_data. */
#include "resident_flight_check.h"
#if RK_LM
#include "lm_restart_routines.h"
#include "lm_kernel_c64_rows.h"
#else
#include "cm_kernel_c64_rows.h"
#endif
static const rk_check_data data={rk_c64_case,rk_c64_digest,RK_C64_ROWS,
#if RK_LM
 &lm_restart_routines
#else
 0
#endif
};
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){(void)selector;
 f->value=(uint16_t)(uintptr_t)&data;return APOLLO_OV2_DONE;}
