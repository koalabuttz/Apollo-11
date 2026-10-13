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
 * vectors, CIA1 timer A at 10 ms, the REU store at offset 0 opened and verified, the runtime
 * initialized on the image at $7000. Returns 0, or 1 (store header) or 2
 * (store contents). Lives in the job slot: call it once, before any module
 * runs. */
uint8_t rk_boot(void);

/* One resident step. First the AGC clock: one 10 ms timer period
 * (rk_timer_tick) per CIA1 interrupt since the last step. Then the highest
 * pending interrupt, if it can be taken here (rk_rupt_take). Then one
 * kernel routine from the image's Z, unless Z is already at stop. At a
 * point with no kernel routine the store's resolver maps the bank (FB with
 * the channel 7 superbank bits, as BBCON holds them) and Z to a module
 * continuation, and the runtime runs it in context 0 (the running job is
 * always core set 0) or 8 (inside an interrupt). A module leaves Z where
 * the program continues. Returns EXEC_OK at stop or after a routine or
 * module ran, otherwise the routine's status or EXEC_UNMAPPED for a point
 * no module resolves. */
enum exec_status rk_resident_step(uint16_t stop_physical);
#endif
