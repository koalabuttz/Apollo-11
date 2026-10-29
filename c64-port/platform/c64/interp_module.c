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
#define IM_MCT  lm_interp_core_mct
#else
#define IM_CORE cm_interp_core
#define IM_MCT  cm_interp_core_mct
#endif
#define IM_SLOTS 64u
#ifdef RK_IRQ_COUNT
/* The resident's IRQ tick count, pinned at RK_IRQ_COUNT (the store links
 * it with --defsym, as it does rk_hardware). */
extern volatile uint16_t ov2_irq_count;
#endif
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 enum exec_status status;
 uint8_t n=0;
 (void)selector;
#ifdef RK_WORK_CLOCK
 /* Work clock: back to the step once the words run reach the next AGC
  * tick, so the step takes it at the next boundary. The core adds each
  * word's yaAGC MCT to IM_MCT as NEWOPS fetches its code word
  * (include/interp_word_mct.h); the first word always runs. A call that
  * ends anything but OK runs no word past the ones it charged: the point
  * is outside the interpreter (after EXIT, RTB or CHANG2), or the core
  * refused it and another holder runs it. The core binds only the image
  * at APOLLO_SHARED_ADDRESS and takes no program. */
 /* In statics, not locals: nothing then has to survive the core call in
  * the zero page the callee may clobber (as locals they cost 75 bytes of
  * saves and reloads). */
 static rk_module_env *im_e;
 static uint8_t im_n;
 im_e = e;
 im_n = 0;
 /* Bit 15 sets once the words run reach the budget (at least 1: the step
  * runs only before its tick is due), and the core then stops before the
  * next word, so the first word always runs. */
 IM_MCT = 0x8000u - im_e->budget;
 do status = IM_CORE(0, (apollo_overlay_state *)APOLLO_SHARED_ADDRESS, im_e->io);
 while (status == EXEC_OK && ++im_n < IM_SLOTS && (int16_t)IM_MCT >= 0);
 if (status == EXEC_UNMAPPED && im_n) status = EXEC_OK;
 im_e->work = (IM_MCT ^ 0x8000u) + im_e->budget;
 im_e->status = status;
 return APOLLO_OV2_DONE;
#elif defined(RK_IRQ_COUNT)
 /* Back to the step once an IRQ tick lands, so the resident takes the
  * AGC interrupt at the next boundary (read twice: no torn count). */
 uint16_t t0,t;
 do t0=ov2_irq_count; while(t0!=ov2_irq_count);
 do{
  status=IM_CORE(e->program,f->state,e->io);
  do t=ov2_irq_count; while(t!=ov2_irq_count);
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
