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
#if RK_CM
/* The CM input channels 30-33 at the values yaAGC starts with
 * (tools/flight_host_run.c). */
__attribute__((used,section(".rk_hardware"))) agc_hardware rk_hardware={.channel30=037777u,.channel31=077777u,.channel32=077777u,.channel33=077777u};
#else
__attribute__((used,section(".rk_hardware"))) agc_hardware rk_hardware;
#endif
/* undo: a non-null marker only. The modules whose io record takes an undo
 * image stash it in the REU (include/flight_undo.h), not at this pointer. */
__attribute__((used, section(".rk_flight"))) flight_io rk_flight = {.undo =
                                                                        (apollo_overlay_state *)1};
uint16_t rk_pending,rk_ticks;
#ifdef RK_RUPT_TRACE
uint8_t rk_rupt_last;
#endif
/* The step profile (platform/c64/dap_pass_check.c -DRK_STEP_PROFILE): the
 * check's rk_prof(n) charges the cycles since its last call to segment n. */
#ifdef RK_STEP_PROFILE
void rk_prof(uint8_t segment);
#define RK_PROF(n) rk_prof(n)
#else
#define RK_PROF(n) ((void)0)
#endif
__attribute__((used,section(".bss.rk_dilation"))) uint32_t rk_dilation;
#ifdef RK_WORK_CLOCK
uint16_t rk_work;
uint8_t rk_tick_phase;
__attribute__((used)) uint8_t rk_cap; /* read by name in rk_realtime_cap */
/* The MCT left before the next tick (at least 1: rk_rupts drained the
 * whole periods at the step's start). */
__attribute__((noinline)) static uint16_t rk_budget(void) { return (uint16_t)(RK_TICK_MCT(rk_tick_phase)-rk_work); }
/* n units of mct each: n added mct times (no multiply on the 6502; mct
 * is a weight, at most 255, and 1 for a module that reports MCT), or for
 * the idle loop (RK_MCT_IDLE), the rest of the tick. */
__attribute__((noinline)) static void rk_charge(uint16_t n,uint8_t mct) {
    uint16_t w=rk_work;
    if(mct==RK_MCT_IDLE) { rk_work=RK_TICK_MCT(rk_tick_phase); return; }
    do w+=n; while(--mct);
    rk_work=w;
}
#ifndef RK_NO_DIRECT
/* The routines a slot module's run may chain (rk_module_limit). */
__attribute__((noinline)) static uint16_t rk_limit(uint8_t mct) { return rk_module_limit(rk_budget(),mct); }
#endif
#endif
rk_module_env rk_env={&rk_program,&rk_device,EXEC_OK,0,0,0};
static uint8_t record[APOLLO_REU_INDEX_RECORD],header[APOLLO_OV2_HEADER_BYTES];
/* The boundary-word table: its REU bank, its address and end (the store
 * image's tail, within one 64 KB bank) and record count. */
static uint16_t boundary_address,boundary_end;
uint16_t rk_boundary_count;
static uint8_t boundary_bank;

/* The kernel's routine tables and SPSIN/SPCOS, filled at boot. */
#if RK_LM
static agc_routine_table basic[5];
#else
static agc_routine_table basic[4];
#endif
agc_native_program rk_program={basic,sizeof basic/sizeof basic[0],0,0,0,0,AGC_TRANSFER_NONE,0,0,rk_fixed_word,0,
                               &rk_hardware};

#ifndef RK_NO_DIRECT
/* rk_program with a loaded module's tables (rk_direct): boot copies
 * rk_program once, and each direct run sets only the table fields
 * (rk_module_program), as nothing else in either changes after boot.
 * Static: absolute addressing is smaller than the soft stack. */
static agc_native_program module_program;
#endif
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
const rk_module_tables *rk_pinned;
static uint8_t BOOT rk_dma(uint8_t command,void *c64,uint32_t reu,uint16_t bytes) {
    return (uint8_t)((apollo_reu_transfer(command,c64,reu,bytes)&(APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))==
        APOLLO_REU_STATUS_END);
}
/* Slot class 2 at $D000 (I/O banked out), or the one class-3 module at
 * $C000 or class-4 module at $E000: plain DMA, BSS zeroed. *user counts the
 * class-3 modules, whose main runs once every module is in (rk_pin_main). */
static uint8_t BOOT rk_load_fixed(void *user,uint32_t reu,const apollo_ov2_dirent *d) {
    uint8_t *at=(uint8_t *)(d->slot==APOLLO_OV2_SLOT_PINNED ? APOLLO_OV2_PINNED_ADDRESS : APOLLO_OV2_TASK_ADDRESS);
    if(d->slot==APOLLO_OV2_SLOT_FIXED)
        return rk_hidden_dma(APOLLO_REU_FETCH,(void *)APOLLO_OV2_FIXED_ADDRESS,reu,d->bytes) &&
               rk_hidden_dma(APOLLO_REU_VERIFY,(void *)APOLLO_OV2_FIXED_ADDRESS,reu,d->bytes);
    if(d->slot==APOLLO_OV2_SLOT_PINNED && (*(uint8_t *)user)++) return 0;
    if(!rk_dma(APOLLO_REU_FETCH,at,reu,d->bytes) || !rk_dma(APOLLO_REU_VERIFY,at,reu,d->bytes)) return 0;
    memset((void *)(uintptr_t)(at[APOLLO_OV2_H_BSS]|(uint16_t)at[APOLLO_OV2_H_BSS+1]<<8),0,
           at[APOLLO_OV2_H_BSS_BYTES]|(uint16_t)at[APOLLO_OV2_H_BSS_BYTES+1]<<8);
    return 1;
}
/* The class-3 module's main, once, to build its table list (which may name
 * the class-4 module's tables too), which must be direct (rk_pinned). */
static uint8_t BOOT rk_pin_main(void) {
    uint8_t *at=(uint8_t *)APOLLO_OV2_PINNED_ADDRESS;
    const rk_module_tables *t;
    apollo_ov2_frame f;
    memset(&f,0,sizeof f);
    f.value=(uint16_t)(uintptr_t)&rk_env; f.state=IMAGE;
    t=(const rk_module_tables *)(uintptr_t)(at[APOLLO_OV2_H_TABLES]|(uint16_t)at[APOLLO_OV2_H_TABLES+1]<<8);
    if(!t || c64_ov2_invoke(0,at,&f,0)!=APOLLO_OV2_DONE || t->ready!=RK_TABLES_DIRECT) return 0;
    rk_pinned=t;
    return 1;
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
#ifndef RK_NO_DIRECT
    module_program=rk_program;
#endif
    if(!apollo_reu_store_open(&rk_store,0,RK_MISSION,RK_SERVICES_VERSION,record)) return 1;
    if(apollo_reu_store_verify(&rk_store,(uint8_t *)RK_BOOT_SCRATCH)!=APOLLO_OV2_OK) return 2;
    apollo_ov2_init(&rk_runtime,apollo_reu_store_backend(&rk_store,c64_ov2_invoke,0),
        (uint8_t *)APOLLO_OV2_JOB_ADDRESS,(uint8_t *)APOLLO_OV2_TASK_ADDRESS,header,IMAGE,
        RK_MISSION,RK_SERVICES_VERSION,0);
    {
        /* The boundary-word table ends the image: its entries, then their
         * count and a zero word (tools/overlay_v2_pack.py --tail). */
        uint32_t end=rk_store.base+((uint32_t)rk_store.pages<<8),first;
        uint16_t tail[2];
        if((apollo_reu_transfer(APOLLO_REU_FETCH,tail,end-sizeof tail,sizeof tail)&
            (APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))!=APOLLO_REU_STATUS_END || !tail[0] || tail[1])
            return 3;
        first=end-sizeof tail-(uint32_t)tail[0]*sizeof(agc_site_operand);
        if((first>>16)!=((end-1u)>>16) || first<rk_store.base+((uint32_t)rk_store.resolver_page<<8)) return 3;
        boundary_bank=(uint8_t)(first>>16); boundary_address=(uint16_t)first; rk_boundary_count=tail[0];
        boundary_end=(uint16_t)(end-sizeof tail);
    }
    /* Then the boot-loaded modules; a class-4 module holds the task slot's
     * RAM, so the runtime has no task-slot module loaded. */
    {
        uint8_t pinned=0;
        if(!apollo_reu_store_fixed(&rk_store,rk_load_fixed,&pinned) || (pinned && !rk_pin_main())) return 4;
        rk_runtime.loaded[APOLLO_OV2_SLOT_TASK]=0;
    }
    *(volatile uint8_t *)RK_STACK_CANARY=RK_STACK_CANARY_VALUE;
    return 0;
}

/* One 4-byte REU fetch into rk_boundary_probe from a table address, by
 * register (as apollo_reu_transfer does, without its 32-bit address
 * arithmetic: the table lies in one 64 KB bank). */
agc_site_operand rk_boundary_probe;   /* DMA target */
static bool boundary_fetch(uint16_t at) {
    volatile uint8_t *reu=(volatile uint8_t *)0xdf00u;
    (void)reu[APOLLO_REU_STATUS];
    reu[APOLLO_REU_ADDR_CONTROL]=0;
    reu[APOLLO_REU_C64_LO]=(uint8_t)(uintptr_t)&rk_boundary_probe; reu[APOLLO_REU_C64_HI]=(uint8_t)((uintptr_t)&rk_boundary_probe>>8);
    reu[APOLLO_REU_REU_LO]=(uint8_t)at; reu[APOLLO_REU_REU_HI]=(uint8_t)(at>>8); reu[APOLLO_REU_REU_BANK]=boundary_bank;
    reu[APOLLO_REU_LEN_LO]=sizeof rk_boundary_probe; reu[APOLLO_REU_LEN_HI]=0;
    reu[APOLLO_REU_COMMAND]=APOLLO_REU_FETCH;
    return (reu[APOLLO_REU_STATUS]&(APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))==APOLLO_REU_STATUS_END;
}
const agc_site_operand *rk_boundary_entry(uint16_t i) {
    if(i>=rk_boundary_count || !boundary_fetch((uint16_t)(boundary_address+i*sizeof rk_boundary_probe))) return 0;
    return &rk_boundary_probe;
}
/* The word at a boundary key, by binary search of the table over its
 * addresses (entries are 4 bytes; boundary_end is one past the last). */
bool rk_fixed_word(const agc_native_program *p, uint16_t physical, agc_word *word) {
    uint16_t low=boundary_address,high=boundary_end;
    (void)p;
    while(low<high) {
        uint16_t middle=(uint16_t)(low+((uint16_t)(high-low)>>1&~3u));
        if(!boundary_fetch(middle)) return false;
        if(rk_boundary_probe.physical==physical) { *word=rk_boundary_probe.word; return true; }
        if(rk_boundary_probe.physical<physical) low=(uint16_t)(middle+4u); else high=middle;
    }
    return false;
}

/* The kernel's seam reports (RK_SEAMS): a job's native seams move with its
 * core set and end with it. */
void rk_seams_exchange(agc_word newjob) {
    uint8_t i;
    /* No division: it would link the 125-byte __udivmodqi4. */
    for(i=0;i<APOLLO_OV2_JOB_CONTEXTS;++i)
        if(newjob==12u*i) { apollo_ov2_seam_exchange(&rk_runtime,0,i); break; }
}
void rk_seams_release(void) { apollo_ov2_seam_release(&rk_runtime,0); }

#ifdef RK_WORK_CLOCK
/* The real-time cap: a tick waits until CIA1 has counted one more
 * 10 ms period than the ticks taken (rk_cap, against
 * ov2_irq_count's low byte), so AGC time never runs ahead of wall time on a
 * fast machine. A lag past RK_CAP_LAG periods is forgiven, so a
 * machine that fell behind does not race later to catch up. A
 * slower-than-real-time C64 never waits here. */
#define RK_CAP_ASM_(n) #n
#define RK_CAP_ASM(n)  RK_CAP_ASM_(n)
/* In assembly: as C, the call or the inlined loop cost the step's
 * register allocation 18-40 bytes more than the loop itself. */
static inline void rk_realtime_cap(void)
{
    /* One instruction group per line. */
    // clang-format off
    __asm__ volatile(
        "1:\n\tlda ov2_irq_count\n\tsec\n\tsbc rk_cap\n\tbeq 1b\n"
        "\tcmp #" RK_CAP_ASM(RK_CAP_LAG) "+1\n\tbcc 2f\n"
        "\tlda ov2_irq_count\n\tsbc #" RK_CAP_ASM(RK_CAP_LAG) "\n\tsta rk_cap\n"
        "2:\n\tinc rk_cap"
        ::: "a", "p"); /* rk_cap is the asm's alone: no memory clobber */
    // clang-format on
}
#endif
/* The AGC clock, one rk_timer_tick per CIA1 interrupt since the last step
 * up to RK_TICK_LAG (the rest dropped into rk_dilation), then the highest
 * pending interrupt if it can be taken at this boundary.
 * The count is read with interrupts held off (and their flag restored: the
 * checks step with it set) so one between its two bytes cannot tear it.
 * Out of line, so the size report counts it with the timers. */
__attribute__((noinline)) static void rk_rupts(void) {
#ifdef RK_WORK_CLOCK
    /* Work clock: one period per RK_TICK_WORK of work since the last. */
    while(rk_work>=RK_TICK_MCT(rk_tick_phase)) {
        rk_realtime_cap();
        rk_work-=RK_TICK_MCT(rk_tick_phase);
        rk_tick_phase=rk_tick_phase==2u ? 0u : (uint8_t)(rk_tick_phase+1u);
        rk_timer_tick(IMAGE,&rk_hardware,&rk_pending);
    }
#else
    uint16_t now;
    uint8_t take=RK_TICK_LAG;
    __asm__ volatile("php\n\tsei" ::: "memory");
    now=ov2_irq_count;
    __asm__ volatile("plp" ::: "memory");
    for(;rk_ticks!=now;++rk_ticks) {
        if(take) { --take; rk_timer_tick(IMAGE,&rk_hardware,&rk_pending); }
        else ++rk_dilation;
    }
#endif
#ifdef RK_RUPT_TRACE
    /* The DAP pass check (platform/c64/dap_pass_check.c): which request
     * this step took, if any. */
    rk_rupt_last=
#endif
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
    if (agc_transfer_next(&rk_program, IMAGE, rk_device.superbank, stop_physical, &call) ==
        stop_physical)
        return EXEC_OK;
    if(!call) return EXEC_UNMAPPED;
#ifdef RK_WORK_CLOCK
    rk_charge(RK_RESIDENT_MCT,1u);
#endif
    return call(&rk_program,IMAGE,&rk_device);
}
__attribute__((noinline)) static enum apollo_ov2_status rk_run_module(apollo_ov2_cont at) {
    apollo_ov2_cont resume;
    enum apollo_ov2_status st;
    run_value=(uint16_t)(uintptr_t)&rk_env;
    rk_env.status=EXEC_OK;
    st=apollo_ov2_run(&rk_runtime,rk_hardware.in_interrupt?APOLLO_OV2_TASK_CONTEXT:0u,at,&run_value,&run_action,&resume);
    return st;
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
/* The high byte of the step's fixed point (Z under FB), or 0xff when Z is
 * erasable, for rk_direct's range checks. Static: absolute addressing is
 * smaller than the soft stack. */
static uint8_t rk_point_hi;
__attribute__((noinline)) enum exec_status rk_direct(uint16_t stop_physical) {
    uint8_t i;
    rk_point_hi =
        (uint8_t)(agc_fixed_physical(IMAGE->words[5], IMAGE->words[4], rk_device.superbank) >> 8);
    /* The pinned module (slot class 3) first, then the job and task slots. */
    for(i=0;i<=APOLLO_OV2_SLOTS;++i) {
        const rk_module_tables *t=rk_pinned;
        uint16_t steps;
        if(i) {
            const uint8_t *h=rk_runtime.slot[i-1u];
            if(!rk_runtime.loaded[i-1u]) continue;
            t=(const rk_module_tables *)(uintptr_t)(h[APOLLO_OV2_H_TABLES]|(uint16_t)h[APOLLO_OV2_H_TABLES+1]<<8);
            /* The loader does not read TABLES; the packer checked it lies in
             * the slot, and it is checked again here before it is followed. */
            if((uint16_t)((uintptr_t)t-(uintptr_t)h-APOLLO_OV2_HEADER_BYTES)>=
               (uint16_t)((i-1u==APOLLO_OV2_SLOT_TASK ? APOLLO_OV2_TASK_BYTES : APOLLO_OV2_JOB_BYTES)-
                         APOLLO_OV2_HEADER_BYTES)) continue;
        }
        if(!t || t->ready!=RK_TABLES_DIRECT) continue;
        /* A point outside the key range the module's tables hold (by high
         * byte; tools/flight_store_build.py) is not its point. The step's
         * point is never stop_physical here: rk_trampoline returned first. */
        if ((uint8_t)(rk_point_hi - t->lo) > t->span) {
            RK_PROF(5);
            continue;
        }
        enum exec_status status;
#ifdef RK_WORK_CLOCK
        uint16_t limit=rk_limit(t->mct);
#else
        const uint16_t limit=RK_MODULE_STEPS;
#endif
        rk_module_program(&module_program,t);
        RK_PROF(4);
        status=agc_transfer_run(&module_program,IMAGE,&rk_device,&rk_device.superbank,stop_physical,limit,&steps);
        /* Not this module's point, or its table refuses the entry: the next
         * slot, then the resolver, may hold the point another way. */
        if(!steps && (status==EXEC_UNMAPPED || status==EXEC_OUT_OF_SCOPE)) { RK_PROF(5); continue; }
        RK_PROF(6);
#ifdef RK_WORK_CLOCK
        rk_charge(steps,t->mct);
#endif
        return rk_module_status(status,steps,limit);
    }
    return EXEC_UNMAPPED;
}
#endif
__attribute__((noinline)) enum exec_status rk_resident_step(uint16_t stop_physical) {
    apollo_overlay_state *s=IMAGE;
    apollo_ov2_cont at;
    enum exec_status status;
    uint16_t loc;
    /* The soft stack ran into the pinned module below it. */
    if(*(volatile uint8_t *)RK_STACK_CANARY!=RK_STACK_CANARY_VALUE) return EXEC_INVALID_ACTION;
    RK_PROF(0);
    rk_rupts();
    RK_PROF(1);
    status=rk_trampoline(stop_physical);
    if(status!=EXEC_UNMAPPED) { RK_PROF(2); return status; }
    RK_PROF(3);
#ifndef RK_NO_DIRECT
    status=rk_direct(stop_physical);
    if(status!=EXEC_UNMAPPED) return status;
    RK_PROF(7);
#endif
    loc=(uint16_t)(s->words[5]&07777u);
    if(!rk_runtime.store.resolve(rk_runtime.store.user,rk_resolver_bank(s->words[4],rk_device.superbank,loc),loc,&at))
        return EXEC_UNMAPPED;
    RK_PROF(8);
#ifdef RK_WORK_CLOCK
    rk_env.budget=rk_budget(); rk_env.work=0; rk_env.scale=1u;
#endif
    if(rk_run_module(at)!=APOLLO_OV2_OK) return EXEC_UNMAPPED;
#ifdef RK_WORK_CLOCK
    rk_charge(rk_env.work,rk_env.scale);
#endif
    RK_PROF(9);
#ifndef RK_NO_DIRECT
    /* A direct-only module's main only builds its tables: the point runs
     * through them in this step. */
    if(rk_env.status==EXEC_UNMAPPED) return rk_direct(stop_physical);
#endif
    return rk_env.status;
}
