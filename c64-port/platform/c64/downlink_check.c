/* SPDX-License-Identifier: MIT */
/* flight-downlink-vice: the downlink word stream on the C64 flight store
 * (tools/flight_downlink_check.py --vice, docs/FLIGHT-DOWNLINK.md). The
 * program boots like the flight resident on the whole-program store, takes
 * the native state tools/flight_host_run.c wrote at a sample point of a
 * flight scenario run with FLIGHT_DOWNRUPT (FLIGHT_SNAPSHOT, DL_SNAP), and
 * with CIA1 interrupts off (no AGC clock period, no other interrupt) takes
 * DL_N DOWNRUPTs one after another: each is set pending (rk_pending), and
 * resident steps run until the resident has taken it and returned from it
 * (in_interrupt on, then off), or a step does not return EXEC_OK, or DL_MAX
 * steps. After each one, channels 34 and 35 and the word-order bit (channel
 * 13 bit 7) go to the REU scratch (DL_SCRATCH), six bytes a DOWNRUPT. The
 * script compares them with yaAGC from the same state
 * (tools/flight_downlink_reference.c).
 *
 * Mailbox (tools/vice_run.py layout): $c001 failures, $c005 the failing
 * status class; $c010 the summary (dl_summary); the records from $c040;
 * $a5 at $c000, written last. */
#include <stdint.h>
#include "resident_flight.h"
#include "resident_rupt.h"
#define SNAP_DATA __attribute__((section(".boot.text.dl_snap")))
#define BOOT      __attribute__((noinline, section(".boot.text.dl_check")))
#include DL_SNAP
#ifndef DL_N
#define DL_N 100u
#endif
#ifndef DL_MAX
#define DL_MAX 2000u
#endif
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
typedef struct {
    uint16_t channel34, channel35;
    uint8_t wo, status;
} __attribute__((packed)) dl_record;
_Static_assert(sizeof(dl_record) == 6u, "records are 6 bytes (tools/flight_downlink_check.py)");
_Static_assert(DL_N * sizeof(dl_record) <= 0x1000u - 0x40u, "the records fit $c040-$cfff");
typedef struct {
    uint32_t steps;
    uint16_t records, start_cs, z, fb;
    uint8_t status, in_interrupt;
} __attribute__((packed)) dl_summary;
#define SUMMARY ((dl_summary *)0xc010)
#define RECORDS ((dl_record *)0xc040)
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
    dl_summary sum;
    dl_record r;
    enum exec_status st = EXEC_OK;
    uint16_t k;
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
    /* The CIA1 interrupts boot took are not AGC time. */
    rk_ticks = ov2_irq_count;
    sum.start_cs = (uint16_t)IMAGE->words[025];
    for (k = 0; k < DL_N && st == EXEC_OK; ++k) {
        uint16_t n;
        uint8_t taken = 0;
        rk_pending |= RK_RUPT_BIT(RK_RUPT_DOWN);
        for (n = 0; n < DL_MAX; ++n) {
            st = rk_resident_step(AGC_TRANSFER_NONE);
            ++sum.steps;
            if (rk_hardware.in_interrupt) taken = 1;
            if (st != EXEC_OK || (taken && !rk_hardware.in_interrupt)) break;
        }
        r.channel34 = rk_flight.channel34;
        r.channel35 = rk_flight.channel35;
        r.wo = (uint8_t)((rk_hardware.channel13 & 0100u) != 0u);
        r.status = (uint8_t)(st != EXEC_OK ? 1u : !taken || rk_hardware.in_interrupt ? 2u : 0u);
        apollo_reu_transfer(APOLLO_REU_STASH, &r, DL_SCRATCH + (uint32_t)k * sizeof r, sizeof r);
        ++sum.records;
        if (r.status) break;
    }
    sum.status = (uint8_t)st;
    sum.z = IMAGE->words[5];
    sum.fb = IMAGE->words[4];
    sum.in_interrupt = (uint8_t)rk_hardware.in_interrupt;
    /* The pinned module at $c000 is done with: the records go after the
     * mailbox. */
    apollo_reu_transfer(APOLLO_REU_FETCH, RECORDS, DL_SCRATCH, sum.records * sizeof r);
    *SUMMARY = sum;
    mail[1] = (uint8_t)(sum.records != DL_N || r.status);
    mail[2] = 0;
    mail[3] = (uint8_t)sum.records;
    mail[4] = (uint8_t)(sum.records >> 8);
    mail[5] = (uint8_t)(st != EXEC_OK ? 2 : r.status ? 3 : 0);
    mail[0] = 0xa5;
    for (;;) {
    }
}
