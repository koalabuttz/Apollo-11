/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident: resident state, the boot sequence (in the job
 * slot) and one resident step (resident_flight.h). */
#include "resident_flight.h"
#include "sp_routines.h"
#include "resident_rupt.h"
#if RK_LM
#include "lm_exec_routines.h"
#else
#include "cm_exec_routines.h"
#endif
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
#define BOOT __attribute__((noinline,section(".boot.text")))

extern void ov2_irq_entry(void);
extern volatile uint16_t ov2_irq_count;
extern void ov2_nmi_entry(void);
uint16_t c64_ov2_invoke(void *,uint8_t *,apollo_ov2_frame *,uint16_t);

apollo_reu_store rk_store;
apollo_ov2_runtime rk_runtime;
rk_io rk_device;
agc_hardware rk_hardware;
uint16_t rk_pending,rk_ticks;
static uint8_t record[APOLLO_REU_INDEX_RECORD],header[APOLLO_OV2_HEADER_BYTES];

/* The kernel's routine tables and SPSIN/SPCOS, filled at boot. */
#if RK_LM
static agc_routine_table basic[5];
#else
static agc_routine_table basic[4];
#endif
agc_native_program rk_program={basic,sizeof basic/sizeof basic[0],0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,&rk_hardware};

uint8_t BOOT rk_boot(void) {
    __asm__ volatile("sei");
    *(volatile uint8_t *)1=0x35;
    *(volatile uint16_t *)0xfffe=(uint16_t)(uintptr_t)ov2_irq_entry;
    *(volatile uint16_t *)0xfffa=(uint16_t)(uintptr_t)ov2_nmi_entry;
    /* CIA1 timer A: one interrupt per AGC 10 ms scaler period. */
    *(volatile uint8_t *)0xdc04=(uint8_t)RK_CIA_LATCH;
    *(volatile uint8_t *)0xdc05=(uint8_t)(RK_CIA_LATCH>>8);
    *(volatile uint8_t *)0xdc0e=0x11;
    rk_ticks=ov2_irq_count;
    __asm__ volatile("cli");
#if RK_LM
    basic[0]=lm_kernel_routines; basic[1]=lm_kernel_job_routines;
    basic[2]=lm_kernel_task_routines; basic[3]=lm_kernel_idle_routines; basic[4]=lm_sp_routines;
    rk_program.operands=lm_exec_site_table(&rk_program.operand_count);
#else
    basic[0]=cm_kernel_routines; basic[1]=cm_kernel_job_routines; basic[2]=cm_kernel_idle_routines;
    basic[3]=cm_sp_routines;
    rk_program.operands=cm_exec_site_table(&rk_program.operand_count);
#endif
    if(!apollo_reu_store_open(&rk_store,0,RK_MISSION,RK_SERVICES_VERSION,record)) return 1;
    if(apollo_reu_store_verify(&rk_store,(uint8_t *)RK_BOOT_SCRATCH)!=APOLLO_OV2_OK) return 2;
    apollo_ov2_init(&rk_runtime,apollo_reu_store_backend(&rk_store,c64_ov2_invoke,0),
        (uint8_t *)APOLLO_OV2_JOB_ADDRESS,(uint8_t *)APOLLO_OV2_TASK_ADDRESS,header,IMAGE,
        RK_MISSION,RK_SERVICES_VERSION,0);
    return 0;
}

/* The kernel's seam reports (RK_SEAMS): a job's native seams move with its
 * core set and end with it. */
void rk_seams_exchange(agc_word newjob) {
    if(newjob<12u*APOLLO_OV2_JOB_CONTEXTS && newjob%12u==0u)
        apollo_ov2_seam_exchange(&rk_runtime,0,(uint8_t)(newjob/12u));
}
void rk_seams_release(void) { apollo_ov2_seam_release(&rk_runtime,0); }

/* The AGC clock, one rk_timer_tick per CIA1 interrupt since the last step,
 * then the highest pending interrupt if it can be taken at this boundary.
 * The count is read twice so an interrupt between its two bytes cannot
 * tear it. Out of line, so the size report counts it with the timers. */
__attribute__((noinline)) static void rk_rupts(void) {
    uint16_t now;
    do now=ov2_irq_count; while(now!=ov2_irq_count);
    while(rk_ticks!=now) { rk_timer_tick(IMAGE,&rk_hardware,&rk_pending); ++rk_ticks; }
    rk_rupt_take(&rk_program,IMAGE,rk_device.superbank,rk_device.interrupt_enabled,&rk_pending);
}

/* One kernel routine, or EXEC_UNMAPPED where there is none. The step
 * returns after each routine so that a pending interrupt is taken at the
 * next native boundary. The trampoline and the module runner each have one
 * caller, so LTO would inline them into the step; keeping them out of line
 * costs a call each and lets the size report
 * (tools/resident_flight_build.py) count them as their own components. */
__attribute__((noinline)) static enum exec_status rk_trampoline(uint16_t stop_physical) {
    agc_routine call;
    if(agc_transfer_point(&rk_program,IMAGE,rk_device.superbank)==stop_physical) return EXEC_OK;
    call=agc_transfer_resolve(&rk_program,IMAGE,rk_device.superbank);
    return call ? call(&rk_program,IMAGE,&rk_device) : EXEC_UNMAPPED;
}
__attribute__((noinline)) static enum apollo_ov2_status rk_run_module(apollo_ov2_cont at) {
    apollo_ov2_cont resume;
    uint16_t value=0;
    uint8_t action;
    return apollo_ov2_run(&rk_runtime,rk_hardware.in_interrupt?APOLLO_OV2_TASK_CONTEXT:0u,at,&value,&action,&resume);
}
__attribute__((noinline)) enum exec_status rk_resident_step(uint16_t stop_physical) {
    apollo_overlay_state *s=IMAGE;
    apollo_ov2_cont at;
    enum exec_status status;
    rk_rupts();
    status=rk_trampoline(stop_physical);
    if(status!=EXEC_UNMAPPED) return status;
    if(!rk_runtime.store.resolve(rk_runtime.store.user,
        (uint16_t)((s->words[4]&076000u)|(rk_device.superbank&0160u)),s->words[5],&at))
        return EXEC_UNMAPPED;
    return rk_run_module(at)==APOLLO_OV2_OK ? EXEC_OK : EXEC_UNMAPPED;
}
