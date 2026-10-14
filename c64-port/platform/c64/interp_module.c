/* SPDX-License-Identifier: MIT */
/* The runtime interpreter's flight module (ABI 2, lane I, $0A01): runs
 * interpretive slots with the compact interpreter (src/lm_interp_core.c,
 * src/cm_interp_core.c) from the point the step resolved to this module
 * until Z leaves the interpreter. The interpreter recognizes its own
 * points, so the module needs no routine tables or transfer-ABI search
 * (platform/c64/flight_table_module.c). Value: the step's rk_module_env;
 * status as the table modules report it. */
#include "resident_flight.h"
#include "interp_runtime.h"
#if RK_LM
#define IM_CORE lm_interp_core
#else
#define IM_CORE cm_interp_core
#endif
#define IM_SLOTS 64u
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 enum exec_status status;
 uint8_t n=0;
 (void)selector;
#ifdef RK_IRQ_COUNT
 /* Back to the step once an IRQ tick lands, so the resident takes the
  * AGC interrupt at the next boundary (read twice: no torn count). */
 volatile const uint16_t *c=(volatile const uint16_t *)RK_IRQ_COUNT;
 uint16_t t0,t;
 do t0=*c; while(t0!=*c);
 do{
  status=IM_CORE(e->program,f->state,e->io);
  do t=*c; while(t!=*c);
 }while(status==EXEC_OK&&++n<IM_SLOTS&&t==t0);
#else
 do status=IM_CORE(e->program,f->state,e->io);
 while(status==EXEC_OK&&++n<IM_SLOTS);
#endif
 /* Leaving the interpreter after at least one slot is where the module
  * hands back; the next step resolves the point. */
 if(status==EXEC_UNMAPPED&&n)status=EXEC_OK;
 e->status=status;
 return APOLLO_OV2_DONE;
}
