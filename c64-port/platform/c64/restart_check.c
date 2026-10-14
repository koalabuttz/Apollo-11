/* SPDX-License-Identifier: MIT */
/* flight-restart-vice: a hardware restart (GOJAM) on the C64 flight store
 * (tools/flight_restart_check.py, docs/FLIGHT-RESTART.md). The program boots
 * like the flight resident on the whole-program store, then takes the native
 * state tools/flight_host_run.c wrote at the sample point where FLIGHT_GOJAM
 * restarted the AGC (FLIGHT_SNAPSHOT: Z 04000, Q the interrupted Z, the
 * hardware GOJAM's channels; RS_SNAP), and runs resident steps from 04000.
 *
 * RS_CS unset (the burst): CIA1 interrupts off, so no AGC clock period and
 * no interrupt, until the AGC is idle at ADVAN (RS_IDLE) again: GOPROG, the
 * phase-table check, STARTSUB/STARTSB2, each active group's restart and the
 * jobs those start, up to where they wait for the clock; or until a step
 * does not return EXEC_OK (a point the store does not hold), or RS_MAX
 * steps. The script compares the image at $7000 with yaAGC run from the
 * same state with its clock frozen (tools/flight_restart_reference.c).
 * RS_CS set (the resume): CIA1 interrupts on, as the flight resident runs,
 * for RS_CS centiseconds of AGC time or until a step does not return
 * EXEC_OK.
 *
 * RS_GOJAM_MODULE (flight-gojam-vice): RS_SNAP is the state at the restart
 * point before the GOJAM (the -pre snapshot), and the program applies the
 * GOJAM on the C64 itself, as a resident trigger would: the GOJAM module
 * (RK_MODULE_GOJAM, platform/c64/resident_gojam_module.c) through
 * rk_run_module, the pending requests cleared, then
 * apollo_ov2_restart_keep_pinned; then the burst as above. With
 * RS_GOJAM_ONLY it stops after the module, so the script can check the
 * image and the device records against the post-GOJAM snapshot
 * (rs_devices at $c040). The program then links resident_flight.c into
 * itself (the script leaves it out of the source list) to reach the
 * resident's own static rk_run_module.
 *
 * Mailbox (tools/vice_run.py layout): $c001 failures, $c005 the failing
 * status class; $c010 the summary (rs_summary); $a5 at $c000, written last. */
#include <stdint.h>
#ifdef RS_GOJAM_MODULE
#include "resident_flight.c"
#undef BOOT
#undef IMAGE
#else
#include "resident_flight.h"
#endif
#define SNAP_DATA __attribute__((section(".boot.text.rs_snap")))
#define BOOT      __attribute__((noinline, section(".boot.text.rs_check")))
#include RS_SNAP
#ifndef RS_MAX
#define RS_MAX 200000ul
#endif
/* ADVAN (01,3214), the idle loop's sample point (tools/flight_host_run.c). */
#define RS_IDLE (1024u + (03214u - 02000u))
#define IMAGE   ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
typedef struct {
    uint32_t steps;
    uint16_t start_cs, end_cs, z, fb, q, newjob;
    uint8_t status, superbank, in_interrupt, interrupt_enabled, idle;
} __attribute__((packed)) rs_summary;
#define SUMMARY ((rs_summary *)0xc010)
#ifdef RS_GOJAM_MODULE
/* After the GOJAM module: rk_hardware's fields in SNAP_HW's order, then
 * rk_flight's in SNAP_FLIGHT's, rk_device's superbank and interrupt latch,
 * the run's apollo_ov2_status and rk_env.status, rk_pending, and the
 * runtime's job and task caches and fault latch after the restart (or
 * before it, with RS_GOJAM_ONLY). */
#define DEVICES         ((volatile uint16_t *)0xc040)
#define RS_DEVICE_WORDS 34u
#endif
extern volatile uint16_t ov2_irq_count;
static volatile uint8_t *const mail = (volatile uint8_t *)0xc000;

static void BOOT load_snapshot(void)
{
    memcpy(IMAGE->words, snap_words, sizeof snap_words);
    SNAP_IO(rk_device)
    SNAP_HW(rk_hardware)
    SNAP_FLIGHT(rk_flight)
    rk_pending = 0;
}

int main(void)
{
    rs_summary sum;
    enum exec_status st = EXEC_OK;
    uint8_t idle = 0;
    if (rk_boot()) {
        mail[1] = 1;
        mail[5] = 1;
        mail[0] = 0xa5;
        for (;;) {
        }
    }
    memset(&sum, 0, sizeof sum);
    load_snapshot();
    __asm__ volatile("sei");
#ifdef RS_GOJAM_MODULE
    {
        const apollo_ov2_cont gojam = {RK_MODULE_GOJAM, 0};
        const agc_hardware *h = &rk_hardware;
        const flight_io *fl = &rk_flight;
        enum apollo_ov2_status run = rk_run_module(gojam);
        uint16_t out[RS_DEVICE_WORDS], n = 0;
        /* The resident trigger's part (docs/FLIGHT-RESTART.md). */
        rk_pending = 0;
#ifndef RS_GOJAM_ONLY
        apollo_ov2_restart_keep_pinned(&rk_runtime);
#endif
        out[n++] = h->channel11;
        out[n++] = h->in_interrupt;
        out[n++] = h->channel5;
        out[n++] = h->channel6;
        out[n++] = h->channel13;
        out[n++] = h->channel15;
        out[n++] = h->channel10;
        out[n++] = h->channel32;
        out[n++] = h->channel12;
        out[n++] = h->channel14;
        out[n++] = h->channel30;
        out[n++] = h->channel33;
        out[n++] = h->channel31;
        out[n++] = h->channel3;
        out[n++] = h->channel4;
        out[n++] = fl->superbank;
        out[n++] = fl->interrupt_enabled;
        out[n++] = fl->channel16;
        out[n++] = fl->channel34;
        out[n++] = fl->channel35;
        out[n++] = fl->trap31a;
        out[n++] = fl->trap31b;
        out[n++] = fl->trap32;
        out[n++] = fl->executive_alarm;
        out[n++] = fl->waitlist_alarm;
        out[n++] = fl->warning;
        out[n++] = rk_device.superbank;
        out[n++] = rk_device.interrupt_enabled;
        out[n++] = (uint16_t)run;
        out[n++] = (uint16_t)rk_env.status;
        out[n++] = rk_pending;
        out[n++] = rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];
        out[n++] = rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];
        out[n++] = rk_runtime.fault;
        for (n = 0; n < RS_DEVICE_WORDS; ++n) DEVICES[n] = out[n];
        if (run != APOLLO_OV2_OK || rk_env.status != EXEC_OK) {
            mail[1] = 1;
            mail[5] = 3;
            mail[0] = 0xa5;
            for (;;) {
            }
        }
#ifdef RS_GOJAM_ONLY
        mail[1] = 0;
        mail[5] = 0;
        mail[0] = 0xa5;
        for (;;) {
        }
#endif
    }
#endif
    /* The CIA1 interrupts boot took are not AGC time. */
    rk_ticks = ov2_irq_count;
    sum.start_cs = (uint16_t)IMAGE->words[025];
#ifdef RS_CS
    __asm__ volatile("cli");
    for (;;) {
        st = rk_resident_step(AGC_TRANSFER_NONE);
        ++sum.steps;
        if (st != EXEC_OK || (uint16_t)((uint16_t)IMAGE->words[025] - sum.start_cs) >= RS_CS) break;
    }
    __asm__ volatile("sei");
#else
    for (;;) {
#ifdef RK_WORK_CLOCK
        rk_work = 0; /* the work clock held too: no tick from the burst's own work */
#endif
        st = rk_resident_step(RS_IDLE);
        ++sum.steps;
        if (st != EXEC_OK) break;
        if ((IMAGE->words[5] & 07777u) == 03214u && ((IMAGE->words[4] >> 10) & 037u) == 1u &&
            !rk_hardware.in_interrupt) {
            idle = 1;
            break;
        }
        if (sum.steps >= RS_MAX) break;
    }
#endif
    sum.end_cs = (uint16_t)IMAGE->words[025];
    sum.status = (uint8_t)st;
    sum.idle = idle;
    sum.z = IMAGE->words[5];
    sum.fb = IMAGE->words[4];
    sum.q = IMAGE->words[2];
    sum.newjob = IMAGE->words[067];
    sum.superbank = (uint8_t)rk_device.superbank;
    sum.in_interrupt = (uint8_t)rk_hardware.in_interrupt;
    sum.interrupt_enabled = (uint8_t)rk_device.interrupt_enabled;
    *SUMMARY = sum;
    mail[1] = (uint8_t)(st != EXEC_OK);
    mail[2] = 0;
    mail[3] = 0;
    mail[4] = 0;
    mail[5] = (uint8_t)(st != EXEC_OK ? 2 : 0);
    mail[0] = 0xa5;
    for (;;) {
    }
}
