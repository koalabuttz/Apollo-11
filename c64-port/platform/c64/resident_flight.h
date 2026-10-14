/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_FLIGHT_H
#define RESIDENT_FLIGHT_H
/* The ABI 2 flight resident (docs/RESIDENT-FLIGHT.md): the thin Executive
 * and Waitlist (include/resident_kernel.h), the transfer-ABI trampoline,
 * the overlay ABI 2 runtime on the REU store, the service table, the
 * interrupt entry and the single-precision subroutines, linked by
 * platform/c64/resident_flight.ld into $0801-$6EFF with the service table at
 * $6F00. Boot-only code (store open and verify, runtime init) links into
 * the job slot at $8000 and runs once, before the first module loads over
 * it. Build with -DRK_LM=1 or -DRK_CM=1 and -DRK_FIXED_IMAGE=1. */
#include <stddef.h>
#include <string.h>
#include "overlay_reu.h"
#include "resident_kernel.h"
#include "flight_io.h"
#if RK_LM
#include "p66_bias.h"
typedef p66_bias_io rk_io;
#define RK_MISSION APOLLO_OV2_LM
#else
#include "cm_native_image.h"
typedef cm_native_io rk_io;
#define RK_MISSION APOLLO_OV2_CM
#endif
/* The service table this resident exports (docs/overlay-v2-services.json);
 * the flight checks build against the first three entries only
 * (tools/resident_flight_build.py CHECK_SERVICES). */
#ifndef RK_SERVICES_VERSION
#define RK_SERVICES_VERSION 0x0203u
#endif
/* Scaled time: AGC time runs at 1/RK_TIME_DIVISOR of wall time (the
 * nightly x0.5 run builds with 2). CIA1 timer A latch for one interrupt per
 * RK_TIME_DIVISOR * 10 ms (PAL, 985,248 Hz: the timer counts latch..0). */
#ifndef RK_TIME_DIVISOR
#define RK_TIME_DIVISOR 1u
#endif
#define RK_CIA_LATCH (9852u*RK_TIME_DIVISOR-1u)
_Static_assert(RK_TIME_DIVISOR>=1u && RK_TIME_DIVISOR<=6u,"RK_CIA_LATCH must fit timer A");
/* The clock ticks one step catches up: when more CIA1 interrupts than this
 * arrived since the last step (the step ran longer than RK_TICK_LAG AGC
 * periods), the rest are dropped and counted in rk_dilation, so AGC time
 * stretches instead of the timers bunching (docs/RESIDENT-FLIGHT.md). */
#ifndef RK_TICK_LAG
#define RK_TICK_LAG 2u
#endif
_Static_assert(RK_TICK_LAG>=1u && RK_TICK_LAG<=255u,"a step takes 1 to 255 ticks");
/* The work clock (docs/RESIDENT-FLIGHT.md), the flight default; -DRK_WALL_CLOCK
 * builds the CIA path above instead. AGC time advances by the work run,
 * not by wall time: each routine adds its module's weight, the yaAGC
 * machine cycles (MCT) one of its routines takes on average (tools/
 * flight_mct.py calibrates them on the host lockstep, docs/flight-mct.json),
 * and every 853 1/3 MCT is one rk_timer_tick (10 ms of AGC time). Module
 * loads, the step's dispatch and the C64's interrupts cost nothing, so the
 * same state and inputs give the same tick schedule at any C64 speed.
 * rk_work counts the MCT since the last tick; the periods are 854, 853 and
 * 853 MCT in turn (rk_tick_phase), so tick k falls at the first whole MCT
 * at or past k * 853 1/3 (2,560 every three). */
#ifndef RK_WALL_CLOCK
#define RK_WORK_CLOCK 1
#define RK_TICK_MCT(phase) (853u+((phase)==0u))
/* The real-time cap's forgiven lag, in 10 ms periods (rk_rupts). */
#ifndef RK_CAP_LAG
#define RK_CAP_LAG 16
#endif
/* Weights from the calibration (tools/resident_flight_build.py): the
 * resident tables', each interrupt-body wrapper's (RK_MODULE_LEAD_INS..
 * RK_MODULE_LM_T6; a wrapper reports the routines it ran, which the step
 * weighs) and the interpreter's per slot. A table module's is FS_MCT. */
#ifndef RK_RESIDENT_MCT
#define RK_RESIDENT_MCT 24u
#endif
#ifndef RK_WRAP_MCT_LIST
#define RK_WRAP_MCT_LIST RK_RESIDENT_MCT,RK_RESIDENT_MCT,RK_RESIDENT_MCT,RK_RESIDENT_MCT
#endif
#ifndef RK_INTERP_MCT
#define RK_INTERP_MCT RK_RESIDENT_MCT
#endif
extern uint16_t rk_work;
extern uint8_t rk_tick_phase;
/* The ticks taken, mod 256, for the real-time cap (rk_rupts). */
extern uint8_t rk_cap;
#endif
/* The dropped ticks, 32 bits at a fixed address below ov2_irq_count (the
 * service table ends at or below it, 61 entries at most), so a VICE run or
 * a module can read how far AGC time fell behind wall time. */
#define RK_DILATION 0x6fb8u
extern uint32_t rk_dilation;
/* The resident's own modules (lane A numbers 0xe0-0xef). */
#define RK_MODULE_BOUNDARY APOLLO_OV2_ID(APOLLO_LANE_A,0xe0)   /* null module (resident_boundary_module.c) */
/* The boundary-word table, which ends the REU store image
 * (tools/overlay_v2_pack.py --tail): its entries (sorted by physical key)
 * and their count, which boot reads; entry i, by REU fetch into
 * rk_boundary_probe (null past the end or on a DMA fault; the next fetch
 * overwrites it). */
extern uint16_t rk_boundary_count;
extern agc_site_operand rk_boundary_probe;
const agc_site_operand *rk_boundary_entry(uint16_t i);
/* What the step hands every module in frame value: the resident program
 * (tables, hardware, fixed_data, site operands) and io. A module that
 * runs AGC routines (the interrupt-body wrappers) reports a routine's
 * failure in status, which the step returns; the step resets it to
 * EXEC_OK before each run. */
typedef struct {
    const agc_native_program *program; void *io; enum exec_status status;
    /* Work clock: the MCT left before the next AGC tick, which the step
     * sets (a module stops chaining routines once its run has used it),
     * and the work the run took, which the module reports, in units of
     * scale MCT (the step sets 1; a wrapper reports the routines it ran
     * and its tables' weight). */
    uint16_t budget, work;
    uint8_t scale;
} rk_module_env;
/* A flight module's routine-table list (platform/c64/flight_table_module.c),
 * named by its header's TABLES word (include/overlay_v2.h). The module
 * builds it on its first run after a load (ready then becomes 1; the load
 * zeroes it with the BSS); from then on the resident calls the module's
 * routines directly from its step, with no ABI 2 run and no module main.
 * The fields up to operand_count have agc_native_program's layout, so one
 * copy puts them in a program; operands and fixed_data are the resident's
 * own where the module has none. */
typedef struct {
    agc_routine_table *basic; uint8_t basic_count;
    agc_routine_table *interpretive; uint8_t interpretive_count;
    const agc_site_operand *operands; uint16_t operand_count;
    bool (*fixed_data)(const agc_native_program *,uint16_t physical,agc_word *word);
    uint8_t ready;   /* 0 not built, RK_TABLES_DIRECT or RK_TABLES_NOT_DIRECT */
    uint8_t mct;     /* work clock: MCT per routine (flight_table_module.c FS_MCT) */
    /* The high byte of the lowest key the tables hold, and how far above
     * it the highest key's is (FS_KEY_LO, FS_KEY_SPAN): 0 and 0xff when the
     * module has interpretive tables (any point). */
    uint8_t lo, span;
} rk_module_tables;
/* Built, and the resident may call the routines straight from the slot
 * (rk_direct); or built, but the tables take an io record other than rk_io
 * (flight_table_module.c FS_IO_KIND), so they run only through the module
 * main, which converts it. */
#define RK_TABLES_DIRECT 1u
#define RK_TABLES_NOT_DIRECT 2u
_Static_assert(offsetof(rk_module_tables,operand_count)==offsetof(agc_native_program,operand_count),
               "rk_module_tables must start like agc_native_program");
/* The resident's program with a module's tables, operands and fixed_data. */
static inline void rk_module_program(agc_native_program *p,const rk_module_tables *t) {
    memcpy(p,t,offsetof(rk_module_tables,fixed_data));
    p->fixed_data=t->fixed_data;
}
/* The pinned module's tables (slot class 3, built at boot), or 0: rk_direct
 * tries them before the job and task slots'. */
extern const rk_module_tables *rk_pinned;
/* Work clock: the weight of the background self-check loop, which the AGC
 * runs only when no job is ready. One run of it (a pass) takes the rest of the
 * tick (rk_charge), so an idle AGC second costs one pass per tick, not the
 * ~85,000 MCT of self-check the hardware fills it with; the step after it
 * takes the tick, where any interrupt that readies a job comes. */
#define RK_MCT_IDLE 0u
/* Routines one module run may chain before it returns to the step. */
#define RK_MODULE_STEPS 256u
/* Work clock: the routines a run may chain within budget (MCT), at most
 * the one that reaches the next tick, so the step takes the tick at the
 * boundary after it, for a run of weight mct each (by subtraction: at
 * most 214 passes, and no division on the 6502). */
static inline uint16_t rk_module_limit(uint16_t budget,uint8_t mct) {
    uint16_t n=1u;
    if(mct==RK_MCT_IDLE) return 4u;   /* one pass: SELFCHK, POSTJUMP, back to ADVAN */
    /* A tick's budget is at most 854 and a weight at least 4
     * (tools/resident_flight_build.py mct_weight): under RK_MODULE_STEPS. */
    while(budget>=mct) { budget-=mct; ++n; }
    return n;
}
/* A module's run, as the step reports it: an unmapped point after at least
 * one routine is where the module hands back (the next step resolves it),
 * and so is the step limit. */
static inline enum exec_status rk_module_status(enum exec_status status,uint16_t steps,uint16_t limit) {
    if(status==EXEC_UNMAPPED && steps) return EXEC_OK;
    if(status==EXEC_OUT_OF_SCOPE && steps==limit) return EXEC_OK;
    return status;
}
extern rk_module_env rk_env;
/* The interrupt-body wrappers (platform/c64/resident_rupt_module.c). */
#define RK_MODULE_LEAD_INS APOLLO_OV2_ID(APOLLO_LANE_A,0xe1)   /* task slot */
#define RK_MODULE_T4RUPT APOLLO_OV2_ID(APOLLO_LANE_A,0xe2)     /* task slot */
#define RK_MODULE_KEYRUPT APOLLO_OV2_ID(APOLLO_LANE_A,0xe3)    /* task slot */
#define RK_MODULE_LM_T6 APOLLO_OV2_ID(APOLLO_LANE_A,0xe4)      /* task slot, LM only */
/* The interpreter's module (platform/c64/interp_module.c). */
#define RK_MODULE_INTERP 0x0a01u
/* The hardware restart (platform/c64/resident_gojam_module.c,
 * docs/FLIGHT-RESTART.md): the GOJAM change to the image and rk_hardware;
 * run with rk_run_module from a resident trigger. */
#define RK_MODULE_GOJAM APOLLO_OV2_ID(APOLLO_LANE_A, 0xe5) /* task slot */
/* The flight program's device fields neither rk_io nor agc_hardware holds
 * (include/flight_io.h flight_io: channels 16, 34, 35, the trap and alarm
 * latches, ...), for the modules whose tables take another io record. Its
 * superbank and interrupt_enabled are copies, refreshed from rk_device
 * around each such run. At RK_FLIGHT_IO. */
#define RK_FLIGHT_IO 0x6fdcu
/* rk_hardware sits at RK_HARDWARE, below rk_flight, so those modules
 * convert with absolute addressing. */
#define RK_HARDWARE 0x6fbeu
/* The CIA1 interrupt count (ov2_irq_count, platform/c64/overlay_v2_irq.S)
 * sits at RK_IRQ_COUNT, below rk_hardware: a long-running module (the
 * interpreter, platform/c64/interp_module.c) returns to the step when it
 * moves, so the step takes the interrupt. Read it twice: an interrupt
 * between its two bytes can tear one read. */
#define RK_IRQ_COUNT 0x6fbcu
extern flight_io rk_flight;
#if RK_LM
#define RK_IO_BASE FLIGHT_IO_P66
#else
#define RK_IO_BASE FLIGHT_IO_CM_NATIVE
#endif
/* 256 bytes of boot scratch for apollo_reu_store_verify (before the
 * pinned module loads over it). */
#define RK_BOOT_SCRATCH 0xc100u
/* The soft stack's last byte above the pinned module (slot class 3, up to
 * $CBFF): boot writes the canary, and a step that finds it changed returns
 * EXEC_INVALID_ACTION before it runs anything. */
#define RK_STACK_CANARY 0xcc00u
#define RK_STACK_CANARY_VALUE 0xa5u

extern apollo_reu_store rk_store;
extern apollo_ov2_runtime rk_runtime;
extern rk_io rk_device;
extern agc_hardware rk_hardware;
extern agc_native_program rk_program;
/* AGC interrupt requests waiting (RK_RUPT_BIT, include/resident_rupt.h),
 * and the CIA1 interrupts the AGC clock has consumed or dropped. */
extern uint16_t rk_pending, rk_ticks;

/* Boot: KERNAL and BASIC out ($01=$35), the resident RAM interrupt
 * vectors, CIA1 timer A at 10 ms, the REU store at offset 0 opened and
 * verified, the runtime initialized on the image at $7000, the
 * boundary-word module (task slot) run once for its table (rk_fixed_word),
 * then the fixed-extension modules (slot class 2) loaded at $D000, the
 * pinned module (slot class 3, at most one) loaded at $C000 and the pinned
 * task region (slot class 4, at most one) at $E000, over the boundary-word
 * module, then the class-3 module's tables built (rk_pinned; they may name
 * class-4 tables), and the stack canary written. Returns 0, or 1 (store
 * header), 2 (store contents), 3 (boundary-word module) or 4 (a
 * fixed-extension or pinned load).
 * Lives in the job slot: call it once, before any module runs. */
uint8_t rk_boot(void);
/* Boot-only (job slot): one REU command with I/O banked out during the DMA,
 * started through $FF00; 1 when it ends without a fault. */
uint8_t rk_hidden_dma(uint8_t command,void *c64,uint32_t reu,uint16_t bytes);

/* One resident step. First the AGC clock: one 10 ms timer period
 * (rk_timer_tick) per CIA1 interrupt since the last step, at most
 * RK_TICK_LAG (the rest are dropped into rk_dilation). Then the highest
 * pending interrupt, if it can be taken here (rk_rupt_take). Then one
 * kernel routine from the image's Z, unless Z is already at stop. At a
 * point with no kernel routine the store's resolver maps the canonical bank
 * (rk_resolver_bank) and Z to a module continuation, and the runtime runs
 * it in context 0 (the running job is always core set 0) or 8 (inside an
 * interrupt), with rk_env as its value. A module leaves Z where the
 * program continues. Returns EXEC_OK at stop or after a routine or module
 * ran, otherwise the routine's status, the module's rk_env.status, or
 * EXEC_UNMAPPED for a point no module resolves. */
enum exec_status rk_resident_step(uint16_t stop_physical);
/* One run of a loaded module's routine at the current point, straight from
 * its slot (EXEC_UNMAPPED: no loaded module holds it). */
enum exec_status rk_direct(uint16_t stop_physical);

/* The canonical resolver bank for 12-bit address loc, the physical bank in
 * FB form: 04000 or 06000 for fixed-fixed, otherwise FB (bits 14-10), plus
 * 0100 where FB is 030-037 and the superbank bit 0100 is set (banks 040-043). */
uint16_t rk_resolver_bank(agc_word fb, agc_word superbank, uint16_t loc);

/* rk_program.fixed_data: the word at a fixed point from the boundary-word
 * table in the REU store (binary search, one 4-byte record read per probe).
 * False for a key the table does not hold, so the interrupt entry does not
 * vector there and the request stays pending. */
bool rk_fixed_word(const agc_native_program *p, uint16_t physical, agc_word *word);
#endif
