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
#include "overlay_reu.h"
#include "resident_kernel.h"
#if RK_LM
#include "p66_bias.h"
typedef p66_bias_io rk_io;
#define RK_MISSION APOLLO_OV2_LM
#else
#include "cm_native_image.h"
typedef cm_native_io rk_io;
#define RK_MISSION APOLLO_OV2_CM
#endif
#define RK_SERVICES_VERSION 0x0201u
/* CIA1 timer A latch for one interrupt per 10 ms (PAL, 985,248 Hz: the
 * timer counts latch..0). */
#define RK_CIA_LATCH 9851u
/* The resident's own modules (lane A numbers 0xe0-0xef). */
#define RK_MODULE_BOUNDARY APOLLO_OV2_ID(APOLLO_LANE_A,0xe0)   /* task slot, data */
/* What the boundary-word module (resident_boundary_module.c) hands boot. */
typedef struct { const agc_site_operand *words; uint16_t count; } rk_boundary_table;
/* What the step hands every module in frame value: the resident program
 * (tables, hardware, fixed_data, site operands) and io. A module that
 * runs AGC routines (the interrupt-body wrappers) reports a routine's
 * failure in status, which the step returns; the step resets it to
 * EXEC_OK before each run. */
typedef struct { const agc_native_program *program; void *io; enum exec_status status; } rk_module_env;
extern rk_module_env rk_env;
/* The interrupt-body wrappers (platform/c64/resident_rupt_module.c). */
#define RK_MODULE_LEAD_INS APOLLO_OV2_ID(APOLLO_LANE_A,0xe1)   /* task slot */
#define RK_MODULE_T4RUPT APOLLO_OV2_ID(APOLLO_LANE_A,0xe2)     /* task slot */
#define RK_MODULE_KEYRUPT APOLLO_OV2_ID(APOLLO_LANE_A,0xe3)    /* task slot */
#define RK_MODULE_LM_T6 APOLLO_OV2_ID(APOLLO_LANE_A,0xe4)      /* task slot, LM only */
/* 256 bytes of boot scratch for apollo_reu_store_verify. */
#define RK_BOOT_SCRATCH 0xc100u

extern apollo_reu_store rk_store;
extern apollo_ov2_runtime rk_runtime;
extern rk_io rk_device;
extern agc_hardware rk_hardware;
extern agc_native_program rk_program;
/* AGC interrupt requests waiting (RK_RUPT_BIT, include/resident_rupt.h),
 * and the CIA1 interrupts the AGC clock has consumed. */
extern uint16_t rk_pending, rk_ticks;

/* Boot: KERNAL and BASIC out ($01=$35), the resident RAM interrupt
 * vectors, CIA1 timer A at 10 ms, the REU store at offset 0 opened and
 * verified, the runtime initialized on the image at $7000, and the
 * boundary-word module run once for its table (rk_fixed_word). Returns 0,
 * or 1 (store header), 2 (store contents) or 3 (boundary-word module).
 * Lives in the job slot: call it once, before any module runs. */
uint8_t rk_boot(void);

/* One resident step. First the AGC clock: one 10 ms timer period
 * (rk_timer_tick) per CIA1 interrupt since the last step. Then the highest
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
