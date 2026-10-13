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
rk_module_env rk_env={&rk_program,&rk_device,EXEC_OK};
static uint8_t record[APOLLO_REU_INDEX_RECORD],header[APOLLO_OV2_HEADER_BYTES];
/* The boundary-word table: its REU bank and address (in its module's
 * image, within one 64 KB bank) and record count. */
static uint16_t boundary_address,boundary_count;
static uint8_t boundary_bank;

/* The kernel's routine tables and SPSIN/SPCOS, filled at boot. */
#if RK_LM
static agc_routine_table basic[5];
#else
static agc_routine_table basic[4];
#endif
agc_native_program rk_program={basic,sizeof basic/sizeof basic[0],0,0,0,0,AGC_TRANSFER_NONE,0,0,rk_fixed_word,0,
                               &rk_hardware};

static enum apollo_ov2_status rk_run_module(apollo_ov2_cont at);
/* The last module run's result value and action (boot reads them). */
static uint16_t run_value;
static uint8_t run_action;

/* One DMA with I/O banked out ($01=$34), for the $D000 extension (slot
 * class 2): the registers are set with I/O in and the command armed without
 * execute-now, then a write to $FF00 starts it (task-slot RAM there; the
 * byte is written back unchanged). Boot-only. */
uint8_t BOOT rk_hidden_dma(uint8_t command,void *c64,uint32_t reu,uint16_t bytes) {
    volatile uint8_t *trigger=(volatile uint8_t *)0xff00u;
    (void)apollo_reu_transfer((uint8_t)(command&~0x10u),c64,reu,bytes);
    __asm__ volatile("sei");
    *(volatile uint8_t *)1=0x34;
    *trigger=*trigger;
    *(volatile uint8_t *)1=0x35;
    __asm__ volatile("cli");
    return (uint8_t)((apollo_reu_peek(APOLLO_REU_STATUS)&(APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))==
        APOLLO_REU_STATUS_END);
}
static uint8_t BOOT rk_load_fixed(void *user,uint32_t reu,const apollo_ov2_dirent *d) {
    void *at=(void *)APOLLO_OV2_FIXED_ADDRESS;
    (void)user;
    return rk_hidden_dma(APOLLO_REU_FETCH,at,reu,d->bytes) && rk_hidden_dma(APOLLO_REU_VERIFY,at,reu,d->bytes);
}
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
    if(!apollo_reu_store_fixed(&rk_store,rk_load_fixed,0)) return 4;
    apollo_ov2_init(&rk_runtime,apollo_reu_store_backend(&rk_store,c64_ov2_invoke,0),
        (uint8_t *)APOLLO_OV2_JOB_ADDRESS,(uint8_t *)APOLLO_OV2_TASK_ADDRESS,header,IMAGE,
        RK_MISSION,RK_SERVICES_VERSION,0);
    {
        /* Run through the step's module runner, which LTO then keeps as
         * the runtime's only caller. */
        apollo_ov2_cont at={RK_MODULE_BOUNDARY,0};
        apollo_ov2_dirent d;
        const rk_boundary_table *t;
        uint32_t first,last;
        if(rk_run_module(at)!=APOLLO_OV2_OK || run_action!=APOLLO_OV2_DONE ||
           !rk_runtime.store.lookup(rk_runtime.store.user,RK_MODULE_BOUNDARY,&d))
            return 3;
        /* The module image sits at its page of the store, loaded at the
         * task slot's base. */
        t=(const rk_boundary_table *)(uintptr_t)run_value;
        first=rk_store.base+((uint32_t)d.page<<8)+(uint16_t)((uintptr_t)t->words-APOLLO_OV2_TASK_ADDRESS);
        last=first+(uint32_t)t->count*sizeof(agc_site_operand)-1u;
        if(!t->count || (first>>16)!=(last>>16)) return 3;
        boundary_bank=(uint8_t)(first>>16); boundary_address=(uint16_t)first; boundary_count=t->count;
    }
    return 0;
}

/* One 4-byte REU fetch per probe, by register (as apollo_reu_transfer does,
 * without its 32-bit address arithmetic). */
bool rk_fixed_word(const agc_native_program *p, uint16_t physical, agc_word *word) {
    uint16_t low=0,high=boundary_count;
    static agc_site_operand probe;   /* DMA target */
    volatile uint8_t *reu=(volatile uint8_t *)0xdf00u;
    (void)p;
    while(low<high) {
        uint16_t middle=(uint16_t)(low+(uint16_t)((high-low)>>1));
        uint16_t at=(uint16_t)(boundary_address+middle*sizeof probe);
        (void)reu[APOLLO_REU_STATUS];
        reu[APOLLO_REU_ADDR_CONTROL]=0;
        reu[APOLLO_REU_C64_LO]=(uint8_t)(uintptr_t)&probe; reu[APOLLO_REU_C64_HI]=(uint8_t)((uintptr_t)&probe>>8);
        reu[APOLLO_REU_REU_LO]=(uint8_t)at; reu[APOLLO_REU_REU_HI]=(uint8_t)(at>>8); reu[APOLLO_REU_REU_BANK]=boundary_bank;
        reu[APOLLO_REU_LEN_LO]=sizeof probe; reu[APOLLO_REU_LEN_HI]=0;
        reu[APOLLO_REU_COMMAND]=APOLLO_REU_FETCH;
        if((reu[APOLLO_REU_STATUS]&(APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))!=APOLLO_REU_STATUS_END) return false;
        if(probe.physical==physical) { *word=probe.word; return true; }
        if(probe.physical<physical) low=(uint16_t)(middle+1u); else high=middle;
    }
    return false;
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
    run_value=(uint16_t)(uintptr_t)&rk_env;
    rk_env.status=EXEC_OK;
    return apollo_ov2_run(&rk_runtime,rk_hardware.in_interrupt?APOLLO_OV2_TASK_CONTEXT:0u,at,&run_value,&run_action,&resume);
}
/* The resolver's canonical bank (docs/OVERLAY_ABI_V2.md): the physical
 * bank in FB form. Fixed-fixed 04000-07777 is bank 2 or 3 whatever FB and
 * the superbank hold; banked, FB alone, plus 0100 for banks 040-043 (FB
 * 030-037 with superbank bit 0100), so superbank 0 and 060 key the same. */
uint16_t rk_resolver_bank(agc_word fb, agc_word superbank, uint16_t loc) {
    if(loc>=04000u) return loc<06000u ? 04000u : 06000u;
    fb&=076000u;
    return (uint16_t)(fb>=060000u && (superbank&0100u) ? fb|0100u : fb);
}
/* A point some loaded flight module holds: its routines run straight from
 * its slot, through its table list (rk_module_tables, the header's TABLES
 * word), with no ABI 2 run and no module main. A module whose list is not
 * built yet (its first run after a load builds it) is left to the resolver.
 * Where two packed tables hold a key, the loaded one runs it. */
#ifndef RK_NO_DIRECT
static agc_native_program module_program;   /* static: absolute addressing is smaller than the soft stack */
__attribute__((noinline)) enum exec_status rk_direct(uint16_t stop_physical) {
    uint8_t i;
    for(i=0;i<APOLLO_OV2_SLOTS;++i) {
        const uint8_t *h=rk_runtime.slot[i];
        const rk_module_tables *t;
        uint16_t steps;
        if(!rk_runtime.loaded[i]) continue;
        t=(const rk_module_tables *)(uintptr_t)(h[APOLLO_OV2_H_TABLES]|(uint16_t)h[APOLLO_OV2_H_TABLES+1]<<8);
        /* The loader does not read TABLES; the packer checked it lies in the
         * slot, and it is checked again here before it is followed. */
        if((uint16_t)((uintptr_t)t-(uintptr_t)h-APOLLO_OV2_HEADER_BYTES)>=
           (uint16_t)(apollo_ov2_slot_bytes(i)-APOLLO_OV2_HEADER_BYTES) || !t->ready) continue;
        enum exec_status status;
        memcpy(&module_program,&rk_program,sizeof module_program);
        rk_module_program(&module_program,t);
        status=agc_transfer_run(&module_program,IMAGE,&rk_device,&rk_device.superbank,stop_physical,RK_MODULE_STEPS,&steps);
        if(status==EXEC_UNMAPPED && !steps) continue;   /* not this module's point */
        return rk_module_status(status,steps,RK_MODULE_STEPS);
    }
    return EXEC_UNMAPPED;
}
#endif
__attribute__((noinline)) enum exec_status rk_resident_step(uint16_t stop_physical) {
    apollo_overlay_state *s=IMAGE;
    apollo_ov2_cont at;
    enum exec_status status;
    uint16_t loc;
    rk_rupts();
    status=rk_trampoline(stop_physical);
    if(status!=EXEC_UNMAPPED) return status;
#ifndef RK_NO_DIRECT
    status=rk_direct(stop_physical);
    if(status!=EXEC_UNMAPPED) return status;
#endif
    loc=(uint16_t)(s->words[5]&07777u);
    if(!rk_runtime.store.resolve(rk_runtime.store.user,rk_resolver_bank(s->words[4],rk_device.superbank,loc),loc,&at))
        return EXEC_UNMAPPED;
    return rk_run_module(at)==APOLLO_OV2_OK ? rk_env.status : EXEC_UNMAPPED;
}
