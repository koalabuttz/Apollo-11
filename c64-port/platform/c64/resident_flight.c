/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident: resident state, the boot sequence (in the job
 * slot) and one resident step (resident_flight.h). */
#include "resident_flight.h"
#include "sp_routines.h"
#include "resident_rupt.h"
#include "resident_cdu.h"
/* The output drives, once a period: the 6502 version of resident_cdu.c's
 * rk_cdu_drive (platform/c64/resident_cdu.S, checked against the C and
 * yaAGC's vectors by tools/resident_cdu_asm_check.c), which stays the host
 * and vector version. Their state lives here. */
rk_cdu_state rk_cdu;
void rk_cdu_drive6502(uint8_t hi);
#define RK_CDU_DRIVE() rk_cdu_drive6502((uint8_t)(rk_hardware.channel14 >> 8))
#ifdef AGC_SITES_ROPE
#include "rope_image.h"
#endif
#if RK_LM
#define RK_ROPE_REU APOLLO_ROPE_REU_LM
#else
#define RK_ROPE_REU APOLLO_ROPE_REU_CM
#endif
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
/* The input channels 30-33 at the values yaAGC starts with
 * (tools/flight_host_run.c): every switch and discrete off (their bits are
 * 1 for off), so a boot from GOJAM (resident_flight_main.c) sees no radar
 * power, mode switch or abort. A run from a snapshot sets them. */
__attribute__((used,section(".rk_hardware"))) agc_hardware rk_hardware={.channel30=037777u,.channel31=077777u,.channel32=077777u,.channel33=077777u};
/* undo: a non-null marker only. The modules whose io record takes an undo
 * image stash it in the REU (include/flight_undo.h), not at this pointer. */
__attribute__((used, section(".rk_flight"))) flight_io rk_flight = {.undo =
                                                                        (apollo_overlay_state *)1};
__attribute__((used, section(".rk_restart_lamp"))) uint8_t rk_restart_lamp;
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
const uint16_t rk_phase_mct[9] = {214u, 213u, 427u, 213u, 213u, 427u, 213u, 214u, 426u};
/* From each step's start to its period's end. */
static const uint16_t rk_phase_rest[9] = {854u, 640u, 427u, 853u, 640u, 427u, 853u, 640u, 426u};
const uint8_t rk_phase_step[9] = {RK_PHASE_T4, RK_PHASE_T3, RK_PHASE_T5, RK_PHASE_T4, RK_PHASE_T3,
                                  RK_PHASE_T5, RK_PHASE_T4, RK_PHASE_T3, RK_PHASE_T5};
/* Set by the idle loop's charge, cleared by rk_rupts (platform/c64/rk_clock.S). */
uint8_t rk_idle;
__attribute__((used)) uint8_t rk_cap; /* read by name in rk_realtime_cap */
/* The MCT left before the next step (at least 1: rk_rupts drained the
 * whole steps at the step's start), in platform/c64/rk_clock.S. */
uint16_t rk_budget(void);
/* n units of mct each: n added mct times (no multiply on the 6502; mct
 * is a weight, at most 255, and 1 for a module that reports MCT), or for
 * the idle loop (RK_MCT_IDLE), the rest of the period, which rk_rupts
 * drops if an interrupt request ends the idle first. */
__attribute__((noinline)) static void rk_charge(uint16_t n,uint8_t mct) {
    uint16_t w=rk_work;
    if (mct == RK_MCT_IDLE) {
        rk_work = rk_phase_rest[rk_tick_phase & 15u]; /* 0-8: a byte index */
        rk_idle = 1;
        return;
    }
    do w+=n; while(--mct);
    rk_work=w;
}
#ifndef RK_NO_DIRECT
/* The routines a slot module's run may chain: rk_module_limit(rk_budget(),
 * mct), in platform/c64/rk_clock.S. */
uint16_t rk_limit(uint8_t mct);
#endif
#endif
rk_module_env rk_env={&rk_program,&rk_device,EXEC_OK,0,0,0};
static uint8_t record[APOLLO_REU_INDEX_RECORD];
#if defined(APOLLO_OV2_STORE_VERIFIED) && !defined(RK_STEP_PROFILE)
/* The verified load() reads no header copy (src/overlay_v2.c). */
#define RK_HEADER_BUFFER 0
#else
static uint8_t header[APOLLO_OV2_HEADER_BYTES];
#define RK_HEADER_BUFFER header
#endif
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
/* The pages (high bytes, mod 64) that hold a key of the kernel's basic
 * tables (rk_boot; rk_program has no interpretive tables): a point in any
 * other page, most of a flight's points, skips the kernel's search. */
static uint8_t kernel_pages[8];
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
    {
        uint8_t t;
        uint16_t i;
        for (t = 0; t < sizeof basic / sizeof basic[0]; ++t)
            for (i = 0; i < basic[t].count; ++i) {
                uint8_t k = (uint8_t)(basic[t].entries[i].physical >> 8), bit = 1u, j;
                for (j = k & 7u; j; --j) bit = (uint8_t)(bit << 1); /* no shift helper */
                kernel_pages[(k >> 3) & 7u] |= bit;
            }
    }
#ifndef RK_NO_DIRECT
    module_program=rk_program;
#endif
    if(!apollo_reu_store_open(&rk_store,0,RK_MISSION,RK_SERVICES_VERSION,record)) return 1;
    if(apollo_reu_store_verify(&rk_store,(uint8_t *)RK_BOOT_SCRATCH)!=APOLLO_OV2_OK) return 2;
#ifdef AGC_SITES_ROPE
    /* The Executive's site words come from the REU rope image
     * (include/agc_transfer.h AGC_SITES_ROPE), which the store check above
     * does not cover: every listed site must read, and the words must sum
     * to what tools/agc_sites_generate.py compiled. */
    {
        const uint16_t *a = (const uint16_t *)(const void *)rk_program.operands;
        uint16_t i, sum = 0, w;
        for (i = 0; i < (uint16_t)(rk_program.operand_count & 037777u); ++i) {
            if (!rk_dma(APOLLO_REU_FETCH, &w, RK_ROPE_REU + 2u * (uint32_t)a[i], 2u)) return 5;
            sum = (uint16_t)(sum + (w & 077777u));
        }
#if RK_LM
        if (sum != lm_exec_site_sum()) return 5;
#else
        if (sum != cm_exec_site_sum()) return 5;
#endif
    }
#endif
    apollo_ov2_init(&rk_runtime,apollo_reu_store_backend(&rk_store,c64_ov2_invoke,0),
        (uint8_t *)APOLLO_OV2_JOB_ADDRESS,(uint8_t *)APOLLO_OV2_TASK_ADDRESS,RK_HEADER_BUFFER,IMAGE,
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
#ifdef RK_DSKY
    rk_dsky_install(); /* before the first module load reuses the boot-only code */
#endif
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
/* The AGC clock, then the highest pending interrupt if it can be taken at
 * this boundary. The work clock steps the timers (rk_timer_phase) for each
 * whole step of work since the last, but takes a request that can be taken
 * before each, so the later steps' requests wait for it (T4RUPT before
 * T3RUPT when both come due in one period, as on the AGC); the idle loop's
 * rest of the period goes when one is taken. The wall clock runs one rk_timer_tick per
 * CIA1 interrupt since the last step up to RK_TICK_LAG (the rest dropped
 * into rk_dilation).
 * The count is read with interrupts held off (and their flag restored: the
 * checks step with it set) so one between its two bytes cannot tear it.
 * Out of line, so the size report counts it with the timers. */
#ifdef RK_WORK_CLOCK
/* rk_rupts (platform/c64/rk_clock.S) comes here when a timer step is due
 * or a request is pending. */
void rk_rupts(void);
__attribute__((noinline, used)) void rk_rupts_due(void) {
    uint8_t got;
#ifdef RK_STEAL
    /* A --replay window of a FLIGHT_STEAL scenario (tools/flight_host_run.c
     * steal): from RK_STEAL_FROM to RK_STEAL_TO (TIME2:TIME1, centiseconds)
     * each T5 step also spends RK_STEAL MCT that no routine runs in, the
     * counter cycles Apollo 11's RR CDU counts took. The time as the step
     * starts, where the host reads it. Replay builds only. */
    const uint32_t cs =
        ((uint32_t)(IMAGE->words[024] & 037777u) << 14) | (IMAGE->words[025] & 037777u);
    const uint8_t steal = cs >= RK_STEAL_FROM && cs < RK_STEAL_TO;
#endif
    for (;;) {
        uint8_t p = rk_tick_phase, t = rk_phase_step[p];
        got = rk_rupt_take(&rk_program, IMAGE, rk_device.superbank, rk_device.interrupt_enabled,
                           &rk_pending);
        if (got || rk_work < RK_PHASE_MCT(p)) break;
        /* Once a period, at TIME5: the real-time cap, then the output drives
         * (resident_cdu.h), at the step the host harness runs them. */
        if (t == RK_PHASE_T5) {
            rk_realtime_cap();
            RK_CDU_DRIVE();
        }
        rk_work -= RK_PHASE_MCT(p);
        rk_tick_phase = p == 8u ? 0u : (uint8_t)(p + 1u);
        rk_timer_phase(IMAGE, &rk_hardware, &rk_pending, t);
#ifdef RK_STEAL
        if (t == RK_PHASE_T5 && steal) rk_work += RK_STEAL;
#endif
    }
    if (got && rk_idle) rk_work = 0;
    rk_idle = 0;
#ifdef RK_RUPT_TRACE
    /* The DAP pass check (platform/c64/dap_pass_check.c): which request
     * this step took, if any. */
    rk_rupt_last = got;
#endif
}
#else
__attribute__((noinline)) static void rk_rupts(void) {
    uint16_t now;
    uint8_t take=RK_TICK_LAG;
    __asm__ volatile("php\n\tsei" ::: "memory");
    now=ov2_irq_count;
    __asm__ volatile("plp" ::: "memory");
    for(;rk_ticks!=now;++rk_ticks) {
        if (take) {
            --take;
            rk_timer_tick(IMAGE, &rk_hardware, &rk_pending);
            RK_CDU_DRIVE(); /* (after the timers, as the host's scaled path) */
        } else
            ++rk_dilation;
    }
#ifdef RK_RUPT_TRACE
    rk_rupt_last=
#endif
    rk_rupt_take(&rk_program,IMAGE,rk_device.superbank,rk_device.interrupt_enabled,&rk_pending);
}
#endif

/* One kernel routine, or EXEC_UNMAPPED where there is none. The step
 * returns after each routine so that a pending interrupt is taken at the
 * next native boundary. The trampoline and the module runner each have one
 * caller, so LTO would inline them into the step; keeping them out of line
 * costs a call each and lets the size report
 * (tools/resident_flight_build.py) count them as their own components. */
#ifndef RK_NO_DIRECT
/* The step's transfer point, as rk_trampoline found it when the kernel has
 * no routine there (else AGC_TRANSFER_NONE), for rk_step_run to hand to
 * rk_direct. Every program the resident runs has rk_program's
 * interpreter_fetch, loc and bankset (rk_module_program copies only
 * tables), so the point is the same for the kernel and every module without
 * interpretive tables (a DANZIG marker there can make it LOC's). Static:
 * absolute addressing is smaller than the soft stack. */
static uint16_t rk_point;
#endif
__attribute__((noinline)) static enum exec_status rk_trampoline(uint16_t stop_physical) {
    agc_routine call;
    uint16_t point = agc_transfer_next_paged(&rk_program, IMAGE, rk_device.superbank, stop_physical,
                                             &call, kernel_pages);
    if (point == stop_physical) return EXEC_OK;
    if (!call) {
#ifndef RK_NO_DIRECT
        rk_point = point;
#endif
        return EXEC_UNMAPPED;
    }
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
#if !RK_LM
/* On the CM channels 11-14 have two copies: rk_device's, which the tables'
 * records (cm_native_io) hold, and rk_hardware's, which the routines that
 * write a channel straight (p->hardware: the T4RUPT IMU code's WOR and
 * WAND, KEYRUPT's DSALMOUT) and the CDU drive use. After each step the copy
 * that changed since the last step gives its value to the other, as
 * src/flight_io.c's flight_io_leave does after each routine on the host.
 * When both changed, the record's stands, the host's rule too (here two
 * routines in one step can each write a different copy, and neither is
 * reliably the later). The merge ends the step, so the next step's timers
 * (rk_timer_phase's TIME6 enable, channel 13) and CDU drive (channel 14)
 * see it. */
agc_word rk_ch_last[4];
static const uint8_t rk_ch_at[4] = {
    offsetof(agc_hardware, channel11), offsetof(agc_hardware, channel12),
    offsetof(agc_hardware, channel13), offsetof(agc_hardware, channel14)};
__attribute__((noinline)) static void rk_channels_merge(void)
{
    agc_word *d = &rk_device.channel11; /* cm_native_io: channels 11-14 first, in order */
    uint8_t i;
    for (i = 0; i < 4u; ++i, ++d) {
        agc_word *h = (agc_word *)((uint8_t *)&rk_hardware + rk_ch_at[i]);
        if (*d != rk_ch_last[i])
            *h = *d;
        else
            *d = *h;
        rk_ch_last[i] = *d;
    }
}
#endif
/* The hardware restart (docs/FLIGHT-RESTART.md, Entry contract), at a
 * step boundary: the GOJAM module's change to the image and rk_hardware,
 * every interrupt request dropped, the RESTART lamp lit, the killed jobs'
 * seams gone. The pinned
 * module, classes 2-4, the AGC clock and the superbank stay. The next step
 * runs the rope's GOJAM code from 04000. */
__attribute__((noinline)) uint8_t rk_gojam(void)
{
    apollo_ov2_restart_keep_pinned(&rk_runtime); /* a latched fault refuses the module */
    if (rk_run_module((apollo_ov2_cont){RK_MODULE_GOJAM, 0}) != APOLLO_OV2_OK) return 1u;
#if !RK_LM
    rk_channels_merge();
#endif
    rk_pending = 0;
    rk_restart_lamp = 1u;
    apollo_ov2_restart_keep_pinned(&rk_runtime);
    return 0u;
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
 * erasable (its point is AGC_TRANSFER_ERASABLE_BASE or above, which no fixed
 * point reaches), for rk_direct's range checks. Static: absolute addressing
 * is smaller than the soft stack. */
static uint8_t rk_point_hi;
#ifdef AGC_TRANSFER_FIND_ASM
/* platform/c64/agc_find.S, the transfer engine's find (agc_transfer.c). */
agc_routine agc_find(const agc_routine_table *tables, uint8_t count, uint16_t physical);
#endif
/* POSTJUMP's point (Z 04701, fixed-fixed). */
#define RK_CM_POSTJUMP (uint16_t)(2048u + (04701u - 04000u))
__attribute__((noinline)) enum exec_status rk_direct(uint16_t stop_physical, uint16_t point)
{
    uint8_t i;
    if (point == AGC_TRANSFER_NONE)
        point = agc_transfer_point(&rk_program, IMAGE, rk_device.superbank);
    rk_point_hi = (uint8_t)(point >> 8);
    if (rk_point_hi >= (uint8_t)(AGC_TRANSFER_ERASABLE_BASE >> 8)) rk_point_hi = 0xffu;
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
#ifdef AGC_TRANSFER_FIND_ASM
        /* A module without interpretive tables runs from the step's point
         * (rk_point) first: if its basic tables do not hold it, the run
         * would take no step, so one find passes the slot over before the
         * run's setup (rk_limit, rk_module_program). */
        if (!t->interpretive_count && point != AGC_TRANSFER_NONE &&
            !agc_find(t->basic, t->basic_count, point)) {
            RK_PROF(5);
            continue;
        }
#endif
        enum exec_status status;
#ifdef RK_WORK_CLOCK
        uint8_t mct = t->mct;
#if RK_LM /* only the LM store packs the idle loop beside interrupt code */
        if (t->idle && !rk_hardware.in_interrupt) mct = RK_MCT_IDLE;
#endif
        uint16_t limit = rk_limit(mct);
#else
        const uint16_t limit=RK_MODULE_STEPS;
#endif
        rk_module_program(&module_program,t);
        RK_PROF(4);
#ifdef RK_POINT_CHECK
        /* Check builds: the point carried over is the module program's own. */
        if (point != AGC_TRANSFER_NONE && !t->interpretive_count &&
            point != agc_transfer_point(&module_program, IMAGE, rk_device.superbank))
            return EXEC_INVALID_ACTION;
#endif
        uint16_t stop = stop_physical;
#if defined(RK_WORK_CLOCK) && !RK_LM
        /* The CM idle loop's run ends at POSTJUMP (Z 04701), as on the host,
         * which takes it as a step of its own (tools/flight_host_run.c): an
         * interrupt that ends the pass's idle returns there, and POSTJUMP
         * idles again, rather than to ADVAN. */
        if (mct == RK_MCT_IDLE && stop == AGC_TRANSFER_NONE && IMAGE->words[5] != 04701u)
            stop = RK_CM_POSTJUMP;
#endif
        status =
            agc_transfer_run_at(&module_program, IMAGE, &rk_device, &rk_device.superbank, stop,
                                limit, &steps, t->interpretive_count ? AGC_TRANSFER_NONE : point);
        /* Not this module's point, or its table refuses the entry: the next
         * slot, then the resolver, may hold the point another way. The
         * transfer engine counts a refused call as a step, so a table
         * refusing its first call (a compact copy that takes only its own
         * callers: POSTJUMP beside the CM idle loop) also moves on. */
        if (status == EXEC_OUT_OF_SCOPE ? steps <= 1u && steps < limit
                                        : status == EXEC_UNMAPPED && !steps) {
            RK_PROF(5);
            /* A refused call ran: the next slot works its point out again. */
            if (steps) point = AGC_TRANSFER_NONE;
            continue;
        }
        RK_PROF(6);
#ifdef RK_WORK_CLOCK
        rk_charge(steps, mct);
#endif
        return rk_module_status(status,steps,limit);
    }
    return EXEC_UNMAPPED;
}
#endif
/* The step after the interrupts: the kernel's routine, a loaded module's,
 * or the resolver's module. */
__attribute__((noinline)) static enum exec_status rk_step_run(uint16_t stop_physical)
{
    apollo_overlay_state *s=IMAGE;
    apollo_ov2_cont at;
    enum exec_status status;
    uint16_t loc;
#ifndef RK_NO_DIRECT
    rk_point = AGC_TRANSFER_NONE;
#endif
    status=rk_trampoline(stop_physical);
    if(status!=EXEC_UNMAPPED) { RK_PROF(2); return status; }
    RK_PROF(3);
#ifndef RK_NO_DIRECT
    status = rk_direct(stop_physical, rk_point);
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
    if (rk_env.status == EXEC_UNMAPPED) return rk_direct(stop_physical, AGC_TRANSFER_NONE);
#endif
    return rk_env.status;
}
#if defined(RK_WORK_CLOCK) && !defined(RK_RUNS_ONLY)
/* The idle loop's two steps, remembered (docs/RESIDENT-FLIGHT.md, Idle
 * memo): the self-check pass from SELFRET's return (CCS SMODE, CHECKNJ,
 * POSTJUMP to ADVAN) and ADVAN with no job ready (DXCH Z back to the
 * pass). With NEWJOB -0 and SMODE +0, each reads only its key (Z, L, EB, BB,
 * SELFRET and the superbank) and writes only its result (the key's words
 * and A, Q, FB): tests/test_idle_memo.c checks both on random images. So a
 * step that starts on the key it started on last time ends on the result it
 * ended on: the resident writes the result and charges the work it charged,
 * without running the step or loading the idle module. The resident check
 * builds (RK_RUNS_ONLY) leave it out. */
#define RK_IDLE_MEMO 1
#if RK_LM
#define RK_MEMO_PASS  03304u /* 43,3304 */
#define RK_MEMO_ADVAN 03214u /* 01,3214 */
#else
#define RK_MEMO_PASS  03306u /* 43,3306 */
#define RK_MEMO_ADVAN 03231u /* 01,3231 */
#endif
/* The words a step reads or writes: the first six are its key, all nine
 * its result. */
#define RK_SELFRET    01361u
#define RK_MEMO_KEY   6u
#define RK_MEMO_WORDS 9u
static agc_word *const rk_memo_at[RK_MEMO_WORDS] = {
    &IMAGE->words[5], &IMAGE->words[1],          &IMAGE->words[3],
    &IMAGE->words[6], &IMAGE->words[RK_SELFRET], &rk_device.superbank,
    &IMAGE->words[0], &IMAGE->words[2],          &IMAGE->words[4]};
/* A memo: the key bytes, the result bytes, the work the step charged (the
 * idle loop's rest of the period, or a count) and whether it holds a step.
 * The step works on rk_memo at a fixed address, which the 6502 reaches in
 * one instruction, and swaps it with rk_memo_next after. */
typedef struct {
    uint8_t in[2u * RK_MEMO_KEY], out[2u * RK_MEMO_WORDS];
    uint16_t work;
    uint8_t idle, ok;
} rk_memo_bytes;
static rk_memo_bytes rk_memo, rk_memo_next;
static uint8_t rk_memo_now[2u * RK_MEMO_WORDS];
__attribute__((noinline)) static void rk_memo_get(void)
{
    uint8_t i;
    for (i = 0; i < RK_MEMO_WORDS; ++i) ((agc_word *)rk_memo_now)[i] = *rk_memo_at[i];
}
__attribute__((noinline)) static enum exec_status rk_memo_run(uint16_t stop_physical)
{
    enum exec_status status;
    uint8_t i;
    rk_memo_get();
    if (rk_memo.ok) {
        for (i = 0; i < sizeof rk_memo.in; ++i)
            if (rk_memo.in[i] != rk_memo_now[i]) goto run;
        for (i = 0; i < RK_MEMO_WORDS; ++i) *rk_memo_at[i] = ((const agc_word *)rk_memo.out)[i];
        if (rk_memo.idle) {
            rk_charge(0u, RK_MCT_IDLE);
#ifdef RK_DSKY
            rk_dsky_idle();
#endif
        } else
            rk_work += rk_memo.work;
        return EXEC_OK;
    }
run:
    for (i = 0; i < sizeof rk_memo.in; ++i) rk_memo.in[i] = rk_memo_now[i];
    rk_memo.work = rk_work;
    status = rk_step_run(stop_physical);
    /* rk_rupts cleared rk_idle at this step's start. */
    rk_memo.idle = rk_idle;
    rk_memo.work = (uint16_t)(rk_work - rk_memo.work);
    rk_memo.ok = status == EXEC_OK;
    rk_memo_get();
    for (i = 0; i < sizeof rk_memo.out; ++i) rk_memo.out[i] = rk_memo_now[i];
    return status;
}
/* The pass and ADVAN alternate, so after each the other's memo comes in
 * (Z, in the key, keeps a memo from answering for the other step). */
__attribute__((noinline)) static enum exec_status rk_memo_step(uint16_t stop_physical)
{
    enum exec_status status = rk_memo_run(stop_physical);
    uint8_t i, t;
    for (i = 0; i < sizeof rk_memo; ++i) {
        t = ((uint8_t *)&rk_memo)[i];
        ((uint8_t *)&rk_memo)[i] = ((uint8_t *)&rk_memo_next)[i];
        ((uint8_t *)&rk_memo_next)[i] = t;
    }
    return status;
}
#endif
uint8_t rk_spin;
__attribute__((noinline)) enum exec_status rk_resident_step(uint16_t stop_physical)
{
    enum exec_status st;
    /* The soft stack ran into the pinned module below it. */
    if (*(volatile uint8_t *)RK_STACK_CANARY != RK_STACK_CANARY_VALUE)
        return EXEC_OUT_OF_SCOPE; /* stop: not a restart (rk_gojam) */
    RK_PROF(0);
    rk_rupts();
    RK_PROF(1);
    /* The TC trap (docs/FLIGHT-RESTART.md, The TC trap). At VERB69 (43,2037
     * in both ropes, V69's TC VERB69; bank 043 is FB 033 with the
     * superbank's bit 0100), outside an interrupt, the AGC runs nothing but
     * TCs, so the step spins there as the idle loop does, to the period's
     * end. The trap's GOJAM comes at the T5 step when the spin was there for
     * the T3 step before it and no interrupt came between, as yaAGC's trap
     * fires at scaler phase 000 after only TCs since phase 020.
     * tools/flight_host_run.c verb69 and spin do the same. */
    {
        uint8_t spin = rk_spin;
        const agc_word *w = IMAGE->words;
        rk_spin = 0;
        if (!rk_hardware.in_interrupt) {
            if (spin & 2u) return rk_gojam() ? EXEC_OUT_OF_SCOPE : EXEC_OK;
            if (w[5] == 02037u && w[4] == 066000u && (rk_device.superbank & 0100u)) {
                IMAGE->words[2] = 02040u; /* Q: TC VERB69's */
                rk_charge(0, RK_MCT_IDLE);
                /* The rest of the period's MCT over 256 (rk_phase_rest): 3
                 * or 2 with its T3 step, 1 without. */
                rk_spin = (uint8_t)(rk_work >> 8);
                st = EXEC_OK;
                goto out;
            }
        }
    }
#ifdef RK_IDLE_MEMO
    /* A flight step (no stop point) of the pass or ADVAN outside an
     * interrupt, with no job ready and SMODE +0. */
    {
        const agc_word *w = IMAGE->words;
        if (stop_physical == AGC_TRANSFER_NONE && !rk_hardware.in_interrupt && w[067u] == 077777u &&
            !w[01362u] &&
            ((w[4] == 066000u && w[5] == RK_MEMO_PASS) ||
             (w[4] == 02000u && w[5] == RK_MEMO_ADVAN))) {
            st = rk_memo_step(stop_physical);
            goto out;
        }
    }
#endif
    st = rk_step_run(stop_physical);
out:
#if !RK_LM
    rk_channels_merge();
#endif
    return st;
}
