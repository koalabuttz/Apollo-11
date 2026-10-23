/* SPDX-License-Identifier: MIT */
/* dap-pass-vice: what the LM DAP costs the flight resident on a C64, per
 * interrupt and as wall time per AGC time (tools/dap_pass_vice.py,
 * /mnt/project-files/steward/cpu-budget.md). The program boots like the
 * flight resident on the whole-program flight store, so the pinned modules
 * (slot classes 3 and 4) and the DAP excursion load as they do in flight;
 * then it takes the native state tools/flight_host_run.c wrote at one
 * sample point (FLIGHT_SNAPSHOT: the erasable image, the superbank, the
 * interrupt enable, agc_hardware and the flight_io channels, DP_SNAP) and
 * runs resident steps with CIA1 interrupts on, as the flight resident does,
 * until DP_CS centiseconds of AGC time have passed or a step does not
 * return EXEC_OK.
 *
 * CIA2's cascaded timers run through the whole run (interrupts and module
 * loads included). Each AGC interrupt the resident takes (RK_RUPT_TRACE) is
 * one record, from the step that takes it to the step that ends it (in
 * interrupt goes off) or takes the next one: the request, the cycles, the steps and the module
 * loads in each slot, so a DAP pass (T5RUPT) is one record and the job
 * module it evicted shows as a load in the steps after it (the background
 * records, request 0, between interrupts).
 *
 * Records go to the REU scratch during the run (the class-3 module holds
 * $C000-$CBFF) and to $C040 at the end, 10 bytes each (DP_MAX at most).
 * Mailbox (tools/vice_run.py layout): $c001-2 failures, $c003-4 records,
 * $c005 the first failing step; $c006 the DP_WATCH words; $c010 the summary (dp_summary); $a5 at
 * $c000, written last. */
#include <stdint.h>
#include "resident_flight.h"
#include "resident_cdu.h"
#define SNAP_DATA __attribute__((section(".boot.text.dp_snap")))
#define BOOT __attribute__((noinline,section(".boot.text.dp_check")))
/* Zeroed harness state in the cassette buffer (resident_flight.ld .cassette,
 * $0384-$03F1, between the step profile and the replay row), not in the
 * resident's RAM: the DP_BUSY build had run 22 bytes into the ABI 2
 * service table (#720). load_snapshot() clears it. */
#define CASSETTE __attribute__((section(".cassette")))
#include DP_SNAP
#ifndef DP_CS
#define DP_CS 100u
#endif
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
typedef struct { uint8_t request, job_loads, task_loads, status; uint16_t steps; uint32_t cycles; } __attribute__((packed)) dp_record;
_Static_assert(sizeof(dp_record)==10u,"records are 10 bytes (tools/dap_pass_vice.py)");
typedef struct {
    uint32_t cycles, dilation, steps;
    uint16_t start_cs, end_cs, records, z, fb, superbank;
    uint8_t status, lost;
} __attribute__((packed)) dp_summary;
#ifdef RK_STEP_PROFILE
/* The step profile (resident_flight.c RK_PROF): cycles and calls per
 * segment, at $CB80 at the end, below it the records. */
#define DP_SEGMENTS 11u
#define DP_TOP 0xcb80u
#define PROFILE ((dp_segment *)DP_TOP)
typedef struct { uint32_t cycles; uint16_t calls; } __attribute__((packed)) dp_segment;
/* The counts live in the cassette buffer ($033C-$03FB, unused here), not
 * in the resident's RAM, which the flight build has no room to spare. */
#define prof      ((dp_segment *)0x033cu)
#define prof_last (*(uint32_t *)(0x033cu + DP_SEGMENTS * sizeof(dp_segment)))
#else
#define DP_TOP 0xcc00u
#endif
/* Records go to the REU scratch (DP_SCRATCH_BYTES, tools/dap_pass_vice.py
 * reads them back from the REU image VICE writes on exit); the first
 * DP_KEEP are also copied below DP_TOP for the memory dump. */
#ifndef DP_SCRATCH_BYTES
#define DP_SCRATCH_BYTES 32768ul
#endif
#define DP_MAX  (DP_SCRATCH_BYTES / sizeof(dp_record))
#define DP_KEEP ((DP_TOP - 0xc040u) / sizeof(dp_record))
#define RECORDS ((dp_record *)0xc040)
#define SUMMARY ((dp_summary *)0xc010)
/* The REU scratch: the store file's unused tail (tools/dap_pass_vice.py). */
extern uint8_t rk_rupt_last;
extern volatile uint16_t ov2_irq_count;
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;

/* CIA2 timer A counts cycles from $FFFF, timer B its underflows; read
 * without stopping them, again if a high byte changed under the reads (A's
 * low byte wrapping: 256 cycles; A wrapping: B; B's low byte wrapping between
 * its two bytes: a 2^24-cycle tear, one in a long run). */
#define CIA2 ((volatile uint8_t *)0xdd00)
static void BOOT timer_start(void){
 CIA2[0x0e]=0;CIA2[0x0f]=0;
 CIA2[0x04]=0xff;CIA2[0x05]=0xff;CIA2[0x06]=0xff;CIA2[0x07]=0xff;
 CIA2[0x0f]=0x51;
 CIA2[0x0e]=0x11;}
static uint32_t now(void){uint8_t b0,b1,a0,a1;
    do {
        b1 = CIA2[0x07];
        b0 = CIA2[0x06];
        a1 = CIA2[0x05];
        a0 = CIA2[0x04];
    } while (a1 != CIA2[0x05] || b0 != CIA2[0x06] || b1 != CIA2[0x07]);
 return ~(((uint32_t)b1<<24)|((uint32_t)b0<<16)|((uint16_t)a1<<8)|a0);}
#ifdef RK_STEP_PROFILE
void rk_prof(uint8_t segment){uint32_t t=now();prof[segment].cycles+=t-prof_last;++prof[segment].calls;prof_last=now();}
#endif
static uint16_t agc_cs(void)
{
    return (uint16_t)IMAGE->words[025];
}
/* The window clock: centiseconds since clock_start(). */
#ifndef DP_SWEEP
/* TIME1 since the start, as before, but held at its last value while
 * TIME2:TIME1 is behind the start. TIMEDIDL (the V70-V73 uplinks, 04,3567)
 * holds TIME2:TIME1 near zero while it adds the uplinked delta. The host
 * steps whole routines and never sees the hold (tools/flight_host_run.c
 * now() counts TIME2:TIME1), but a C64 step can end in it, and the 14-bit
 * TIME1 difference then reads as a full window. The delta itself counts, as
 * on the host. A negative delta (V70 can move the clock back) that leaves
 * TIME2:TIME1 behind the start would hold the clock until it catches up,
 * so the window would run on past its end: no scenario uplinks one. */
static uint16_t clock_t2 CASSETTE, clock_t1 CASSETTE, clock_last CASSETTE;
static void clock_start(void)
{
    clock_t2 = IMAGE->words[024];
    clock_t1 = IMAGE->words[025];
    clock_last = 0;
}
__attribute__((noinline)) static uint16_t window_cs(void)
{
    uint16_t l = IMAGE->words[025];
    if (((uint32_t)IMAGE->words[024] << 16 | l) < ((uint32_t)clock_t2 << 16 | clock_t1))
        return clock_last;
    return clock_last = (l - clock_t1) & 037777u;
}
#else
/* The DSKY sweep keeps TIME1 since the case's start (Lane C's #674 runs
 * on it). */
static uint16_t clock_t0 CASSETTE;
static void clock_start(void)
{
    clock_t0 = (uint16_t)IMAGE->words[025];
}
static uint16_t window_cs(void)
{
    return (uint16_t)(IMAGE->words[025] - clock_t0) & 037777u;
}
#endif
#ifdef DP_KEYS
/* The crew's DSKY keys in the window (DP_KEYS, written by
 * tools/flight_scenario_vice.py --keys from the scenario's key string): key
 * k is pressed at the first step at or after dp_key_cs[k] centiseconds into
 * the window, as tools/flight_host_run.c presses it: channel 15 and a
 * KEYRUPT1 request, or for a mark key (bit 0400) channel 16 and KEYRUPT2.
 * The key stays in its channel until the next one, as on the host. */
#include "resident_rupt.h"
#include DP_KEYS
static uint8_t key_next CASSETTE;
static void press_due(uint16_t elapsed)
{
    uint16_t k;
    if (key_next >= DP_NKEYS || elapsed < dp_key_cs[key_next]) return;
    k = dp_key_code[key_next++];
    __asm__ volatile("sei");
    if (k & 0400u) {
        rk_flight.channel16 = (agc_word)(k & 0377u);
        rk_pending |= RK_RUPT_BIT(RK_RUPT_KEY2);
    } else {
        rk_hardware.channel15 = (agc_word)k;
        rk_pending |= RK_RUPT_BIT(RK_RUPT_KEY1);
    }
#if !RK_LM
    /* On the CM, rk_device (cm_native_io) holds the keys too: each store
  * module on a CM record writes its main_keys and navigation_keys back to
  * channels 15 and 16 (platform/c64/flight_table_module.c). */
    rk_device.main_keys = rk_hardware.channel15;
    rk_device.navigation_keys = rk_flight.channel16;
#endif
    __asm__ volatile("cli");
}
#endif

#ifdef DP_REPLAY
/* The host run's inputs over the window and its end state
 * (tools/flight_host_run.c FLIGHT_REPLAY, tools/flight_scenario_vice.py
 * --replay). DP_REPLAY_ROWS 10-byte rows at DP_REPLAY_REU in the REU, outside
 * the flight store and the resident: the centiseconds since the snapshot,
 * kind, where (0 at a sample point, 1 at any step boundary), address and
 * value, the address a C64 one (tools/flight_scenario_vice.py c64_rows
 * turns the host's erasable words, channels and requests into the
 * addresses of the image, rk_hardware, rk_flight, rk_device's copies and
 * rk_pending). Kinds: 1 a signed count added to a counter word (a PIPA),
 * 3 bits ORed in (interrupt requests), 4 a byte stored, else a word
 * stored. A row is applied before the first step at or after its time where
 * it may be (rows go in order: a sample point's row holds back the
 * boundary rows after it; a PIPA count the C64 is late for goes in at any
 * step, and every late row goes in before a state is written). The sample
 * point is the host's: Z at ADVAN in bank 01, NEWJOB -0, no interrupt
 * running or requested; with DP_BUSY also
 * a step boundary outside an interrupt with none requested DP_BUSY
 * centiseconds after the last (FLIGHT_BUSY_SAMPLE). At the first sample
 * point DP_REPLAY_CS or more centiseconds in, before that point's rows,
 * the window ends: the centiseconds, the erasable words, rk_hardware and
 * rk_flight (RK_HARDWARE up to RK_RESTART_LAMP) and 32 bytes of rk_device
 * go to DP_END in the REU, after the same state at the first sample point at or
 * after each centisecond the host wrote a checkpoint at (DP_STATE_FIRST, then
 * the 2-byte table at DP_STATE_TABLE, ended by 0xFFFF; DP_REPLAY_CS is the
 * host's own end, so both sides end at the same point); the rows applied to $c030, the
 * end's centiseconds to $c032, $c034 1 if the end was reached and $c035
 * the states written. */
#include "resident_rupt.h"
#if RK_LM
#define DP_ADVAN 03214u
#else
#define DP_ADVAN 03231u
#endif
typedef struct {
    uint16_t cs;
    uint8_t kind, where;
    uint16_t addr, value, work;
} __attribute__((packed)) dp_row;
_Static_assert(sizeof(dp_row) == 10u, "rows are 10 bytes (tools/flight_host_run.c rp_row)");
#ifdef DP_SWEEP
/* The DSKY sweep (tools/dsky_sweep_vice.py): many windows from one
 * snapshot in one run. At DP_SWEEP in the REU: the case count, then one
 * dp_case each (its rows, its window and where its end state goes; the run
 * writes back whether it ended, the step status and the rows applied).
 * Before the first case the snapshot (the erasable words, rk_hardware and
 * rk_flight, rk_device) is stashed at DP_SWEEP_SNAP; each later case starts
 * from it. A case ends where a window ends, at a step that does not return
 * EXEC_OK, or DP_CS centiseconds in; the next one then starts. Only the
 * window's end state is written (DP_STATE_FIRST 0xFFFF: no checkpoints). */
typedef struct {
    uint32_t rows_at;
    uint16_t rows, cs;
    uint32_t end_at;
    uint8_t ended, status;
    uint16_t applied;
} __attribute__((packed)) dp_case;
_Static_assert(sizeof(dp_case) == 16u, "cases are 16 bytes (tools/dsky_sweep_vice.py)");
static dp_case sweep_case;
static uint16_t sweep_n, sweep_i;
static uint32_t sweep_at = DP_SWEEP + 2ul;
#define DP_REPLAY_CS sweep_case.cs
#endif
/* The row being applied sits in the cassette buffer ($03F2-$03FB; the step
 * profile uses its bottom), not in the resident's RAM. */
#define row (*(dp_row *)0x03f2u)
/* The centisecond the C64 is in. */
static uint16_t replay_now CASSETTE;
/* A boundary row in that centisecond waits for its place in it (where
 * bits 1-2: the work clock's step, 1 the TIME1/TIME3 step's, then 2 and 3;
 * work the MCT since that step), so a DAP pass reads the CDUs on the
 * host's side of it. rk_tick_phase's step: 1 for 2, 5, 8; 2 for 0, 3, 6;
 * 3 for 1, 4, 7. */
/* The row code runs from $0200 (the flight program's DSKY code region,
 * resident_flight.ld .dsky: free in this program), which load_snapshot
 * fills from its load copy, not from the resident's RAM. */
#define LOW_TEXT(name) __attribute__((noinline, section(".dsky.text." name)))
extern uint8_t __rk_dsky_start[], __rk_dsky_end[], __rk_dsky_load[];
#define LOW_DATA __attribute__((section(".dsky.text.dp_steps")))
LOW_DATA static const uint8_t phase_step[9] = {2, 3, 1, 2, 3, 1, 2, 3, 1};
LOW_TEXT("dp_early") static uint8_t row_early(void)
{
    uint8_t at = row.where >> 1 & 3u, step = phase_step[rk_tick_phase];
    return at && row.cs == replay_now && (step < at || (step == at && rk_work < row.work));
}
static uint16_t rows_left = DP_REPLAY_ROWS;
static uint8_t row_ready CASSETTE;
static uint32_t row_at = DP_REPLAY_REU, state_at = DP_END;
#ifdef DP_BUSY
static uint16_t busy_last CASSETTE;
#endif
#if !defined(DP_STATE_FIRST) || !defined(DP_STATE_TABLE)
#error "DP_REPLAY needs DP_STATE_FIRST and DP_STATE_TABLE, the host's checkpoint centiseconds"
#endif
/* The states DP_END has room for (tools/flight_scenario_vice.py
 * max_states): the checkpoints stop one short of it, so the end always has
 * its place and nothing goes past the area. */
#ifndef DP_STATES_MAX
#error "DP_REPLAY needs DP_STATES_MAX, the states DP_END has room for"
#endif
_Static_assert(DP_STATES_MAX >= 1u && DP_STATES_MAX <= 255u, "the states written are counted in a byte ($c035)");
#define DP_IO_BYTES (RK_RESTART_LAMP - RK_HARDWARE)
static uint16_t state_next = DP_STATE_FIRST;
static uint8_t states CASSETTE;
/* The window clock since the snapshot: a window is at most 16,383 cs. */
static uint16_t replay_cs(void)
{
    return window_cs();
}
static uint8_t sample_point(uint16_t t)
{
    if (rk_pending || rk_hardware.in_interrupt) return 0;
    if ((IMAGE->words[5] & 07777u) == DP_ADVAN && (IMAGE->words[4] >> 10) == 1u &&
        (IMAGE->words[067] & 077777u) == 077777u)
        return 1;
#ifdef DP_BUSY
    if ((uint16_t)(t - busy_last) >= DP_BUSY) return 1;
#endif
    (void)t;
    return 0;
}
/* stash and replay_state run from the low region too (LOW_TEXT): with
 * DP_BUSY and RK_STEAL the resident's RAM has no room for them. */
LOW_TEXT("dp_stash") static void stash(void *from, uint16_t n)
{
    apollo_reu_transfer(APOLLO_REU_STASH, from, state_at, n);
    state_at += n;
}
/* The state at a sample point: its centiseconds, the erasable words,
 * rk_hardware and rk_flight, then 32 bytes of rk_device. */
LOW_TEXT("dp_state") static void replay_state(uint16_t t)
{
    static uint16_t at CASSETTE;
    at = t;
    ++states;
    stash(&at, 2u);
    stash(IMAGE->words, 2048u * sizeof(agc_word));
    stash((void *)RK_HARDWARE, DP_IO_BYTES);
    stash(&rk_device, 32u);
}
/* Apply the rows due by t (sample point rows only when sp). */
__attribute__((noinline)) static void replay_rows(uint16_t t, uint8_t sp)
{
    while (rows_left) {
        uint16_t *w;
        if (!row_ready) {
            apollo_reu_transfer(APOLLO_REU_FETCH, &row, row_at, sizeof(dp_row));
            row_at += sizeof(dp_row);
            row_ready = 1;
        }
        /* A sample point's row waits for a sample point, except an overdue
         * PIPA count: the PIPAs count whether or not the C64 is idle, and a
         * READACCS on a C64 that missed the host's sample point would read
         * fewer counts than the host's. */
        if (row.cs > t || (!(row.where & 1u) && !sp && (row.kind != 1u || row.cs >= t)) ||
            row_early())
            return;
        w = (uint16_t *)row.addr;
        __asm__ volatile("sei");
        if (row.kind == 1) {
            /* tools/flight_host_run.c pipa_add: a signed count into a ones'
             * complement counter, held at +-037777. */
            int16_t v = (int16_t)((*w & 040000u) ? -(int16_t)((~*w) & 037777u) : (int16_t)*w) +
                        (int16_t)row.value;
            if (v > 037777) v = 037777;
            if (v < -037777) v = -037777;
            *w = v < 0 ? (uint16_t)((~(uint16_t)(-v)) & 077777u) : (uint16_t)v;
        } else if (row.kind == 3)
            *w |= row.value;
        else if (row.kind == 4)
            *(uint8_t *)w = (uint8_t)row.value;
        else
            *w = row.value;
        __asm__ volatile("cli");
        row_ready = 0;
        --rows_left;
    }
}
/* The rows due before this step; 1 when the window ends here. */
__attribute__((noinline)) static uint8_t replay_due(void)
{
    uint16_t t = replay_cs();
    uint8_t sp = sample_point(t);
    replay_now = t;
    if (sp) {
#ifdef DP_BUSY
        busy_last = t;
#endif
        /* The host writes a state at a sample point after every row before
         * it and before that point's own rows. Rows a busy C64 held back
         * (a host sample point the C64 was not idle at) go in first, or a
         * PIPA count the host has is missing from the state. */
        if (t) replay_rows((uint16_t)(t - 1u), 1u);
        if (t >= DP_REPLAY_CS) {
            replay_state(t);
            return 1;
        }
        if (t >= state_next && states < DP_STATES_MAX - 1u) {
            /* The next host checkpoint: table entry `states` - 1 after
             * this one is written (entry 0 follows DP_STATE_FIRST). */
            replay_state(t);
            apollo_reu_transfer(APOLLO_REU_FETCH, &state_next, DP_STATE_TABLE + 2ul * (states - 1u), 2u);
        }
    }
    replay_rows(t, sp);
    return 0;
}
#endif

extern char __dp_cassette_start[], __dp_cassette_end[];
static void BOOT load_snapshot(void){
    memset(__dp_cassette_start, 0, (size_t)(__dp_cassette_end - __dp_cassette_start));
    memcpy(IMAGE->words, snap_words, sizeof snap_words);
#ifdef DP_REPLAY
    memcpy(__rk_dsky_start, __rk_dsky_load, (size_t)(__rk_dsky_end - __rk_dsky_start));
#endif
#ifdef RK_STEP_PROFILE
 memset(prof, 0, DP_SEGMENTS * sizeof(dp_segment));
#endif
 SNAP_IO(rk_device)
 SNAP_HW(rk_hardware)
 SNAP_FLIGHT(rk_flight)
 SNAP_CDU(rk_cdu)
 rk_device_from_canonical();
 rk_pending = 0;
#ifdef DP_REPLAY
 /* The host's replay runs on the work clock from its first step. */
 rk_work = 0;
 rk_tick_phase = 0;
 rk_part_cont = rk_part_prev = 0;
#endif
}

static dp_record r CASSETTE;
static dp_summary sum CASSETTE;
#ifdef DP_SWEEP
/* The snapshot's state (rk_hardware, rk_flight and the erasable words, one
 * run from RK_HARDWARE to the image's end, then rk_device) to or from the
 * REU. */
__attribute__((noinline)) static void sweep_snap(uint8_t how)
{
    apollo_reu_transfer(how, (void *)RK_HARDWARE, DP_SWEEP_SNAP, 0x8000u - RK_HARDWARE);
    apollo_reu_transfer(how, &rk_device, DP_SWEEP_SNAP + 0x8000u - RK_HARDWARE, sizeof rk_device);
}
/* The first call stashes the snapshot and starts case 0. Later calls write
 * case sweep_i's outcome back to its dp_case and start the next case from
 * the snapshot; 0 after the last. */
__attribute__((noinline)) static uint8_t sweep_next(uint8_t ended, uint8_t status)
{
    if (!sweep_n) {
        apollo_reu_transfer(APOLLO_REU_FETCH, &sweep_n, DP_SWEEP, 2u);
        sweep_snap(APOLLO_REU_STASH);
    } else {
        sweep_case.ended = ended;
        sweep_case.status = status;
        sweep_case.applied = sweep_case.rows - rows_left;
        apollo_reu_transfer(APOLLO_REU_STASH, &sweep_case, sweep_at, 16u);
        sweep_at += 16u;
        if (++sweep_i >= sweep_n) return 0;
        __asm__ volatile("sei");
        sweep_snap(APOLLO_REU_FETCH);
        SNAP_CDU(rk_cdu) /* (outside the stashed range) */
    }
    apollo_reu_transfer(APOLLO_REU_FETCH, &sweep_case, sweep_at, 16u);
    __asm__ volatile("sei");
    rk_pending = 0;
    rk_work = 0;
    rk_tick_phase = 0;
    rk_part_cont = rk_part_prev = 0;
    rk_ticks = ov2_irq_count;
    __asm__ volatile("cli");
    clock_start();
    rows_left = sweep_case.rows;
    row_at = sweep_case.rows_at;
    state_at = sweep_case.end_at;
    row_ready = 0;
    states = 0;
    state_next = DP_STATE_FIRST;
    replay_now = 0;
#ifdef DP_BUSY
    busy_last = 0;
#endif
    sum.start_cs = agc_cs();
    return 1;
}
#endif

__attribute__((unused)) static void close_record(uint32_t cycles, enum exec_status st)
{
    r.cycles = cycles;
    r.status = (uint8_t)st;
    if (sum.records < DP_MAX) {
        apollo_reu_transfer(APOLLO_REU_STASH, &r, DP_SCRATCH + (uint32_t)sum.records * sizeof r,
                            sizeof r);
        ++sum.records;
    } else
        sum.lost = 1;
    r.steps = 0;
    r.job_loads = 0;
    r.task_loads = 0;
    r.request = 0;
}

#ifdef DP_REPLAY
static uint8_t replay_ended CASSETTE;
#endif

#ifdef DP_HW_CHECK
/* The hardware check disk (tools/flight_disk.py, the "test" set;
 * docs/RUN-ON-C64.md, Checking real hardware): a DP_REPLAY window whose
 * report is on the screen, for a photograph. After the store check it
 * prints the video standard, the REU's size and its DMA speed both ways;
 * at the window's end the C64 time, the speed against real time and a
 * CRC of the end state (the erasable words tools/flight_scenario_vice.py
 * compares, the timers and the centiseconds), with PASS when it is the CRC
 * VICE's run of the same program gave (DP_HW_EXPECT; 0 in that first
 * run), and the host run's CRC (DP_HW_HOST) beside it. The CRC and the
 * verdict also go to $c036-$c038, before the mailbox's $a5:
 * tools/flight_disk.py reads them, and its run from the disk stops at
 * dp_done.
 *
 * The resident has no room for this, so all of it is boot-only code: boot
 * stashes the boot code in the REU at DP_HW_SAVE (spare bytes below the
 * rope) before the window's module loads overwrite it, and the window's
 * end fetches it back. Its text and variables are boot-only data too, and it uses no
 * multiply or divide helpers, which would link into the resident. */
#define SCREEN ((volatile uint8_t *)0x0400)
#define COLOUR ((volatile uint8_t *)0xd800)
#define VIC    ((volatile uint8_t *)0xd000)
#define REU    ((volatile uint8_t *)0xdf00)
#define TEXT(name, text)                                                                           \
    static const char name[] __attribute__((section(".boot.text.hw_text"))) = text
TEXT(t_title, "APOLLO 11 ON THE C64: HARDWARE CHECK\n\n");
TEXT(t_pal, "VIDEO: PAL, 985248 HZ\n");
TEXT(t_ntsc, "VIDEO: NTSC, 1022727 HZ\n");
TEXT(t_reu, "REU: ");
TEXT(t_kb, " KB\n");
TEXT(t_kb_more, " KB OR MORE\n");
TEXT(t_dma, "REU DMA, BYTES/S: C64>REU ");
TEXT(t_dma2, "\n                 REU>C64 ");
TEXT(t_run, "\n\nRUNNING " DP_HW_WINDOW "...\n\n");
TEXT(t_agc, "AGC TIME: ");
TEXT(t_cs, " CS IN ");
TEXT(t_steps, " STEPS\nC64 TIME: ");
TEXT(t_s, " S, ");
TEXT(t_x, " X REAL TIME\n\nEND STATE CRC ");
TEXT(t_vice, " (VICE ");
TEXT(t_host, ")\nHOST RUN'S CRC ");
TEXT(t_same, " (THE SAME)");
TEXT(t_differ, " (NOT YET THE SAME)");
TEXT(t_pass, "\n\nPASS: THIS C64 COMPUTES WHAT VICE DOES");
TEXT(t_fail, "\n\nFAIL: THIS C64 DIFFERS FROM VICE");
TEXT(t_status, " (STEP STATUS ");
static const uint32_t hw_ten[10] __attribute__((section(".boot.text.hw_text"))) = {
    1000000000ul, 100000000ul, 10000000ul, 1000000ul, 100000ul, 10000ul, 1000ul, 100ul, 10ul, 1ul};
extern char __rk_boot_start[], __rk_boot_end[];
/* Boot-only data too, kept by the stash (taken once these are set). */
#define HW_DATA __attribute__((section(".boot.text.hw_data")))
static uint16_t hw_at HW_DATA = 0; /* the next screen cell */
static uint8_t hw_col HW_DATA = 0; /* and its column */
static uint32_t hw_hz HW_DATA = 0; /* the C64's clock: PAL or NTSC */
static uint16_t hw_end_crc HW_DATA = 0;
static uint8_t hw_pass HW_DATA = 0;
static void BOOT hw_putc(char c)
{
    uint8_t k = (uint8_t)c;
    if (k == '\n') {
        hw_at = (uint16_t)(hw_at + 40u - hw_col);
        hw_col = 0;
        return;
    }
    if (k >= 0x40u) k = (uint8_t)(k - 0x40u); /* upper case to screen codes */
    SCREEN[hw_at] = k;
    COLOUR[hw_at] = 1u;
    ++hw_at;
    ++hw_col;
}
static void BOOT hw_puts(const char *s)
{
    while (*s) hw_putc(*s++);
}
static uint32_t BOOT hw_div(uint32_t n, uint32_t d)
{
    uint32_t q = 0, r = 0;
    uint8_t i;
    for (i = 0; i < 32u; ++i) {
        r = r << 1 | n >> 31;
        n <<= 1;
        q <<= 1;
        if (r >= d) {
            r -= d;
            q |= 1u;
        }
    }
    return q;
}
static uint32_t BOOT hw_mul(uint32_t a, uint16_t b)
{
    uint32_t p = 0;
    for (; b; b >>= 1, a <<= 1)
        if (b & 1u) p += a;
    return p;
}
static void BOOT hw_putu(uint32_t v)
{
    uint8_t i, d, any = 0;
    for (i = 0; i < 10u; ++i) {
        for (d = 0; v >= hw_ten[i]; ++d) v -= hw_ten[i];
        if (d || any || i == 9u) {
            hw_putc((char)('0' + d));
            any = 1;
        }
    }
}
/* v hundredths, as units and two decimals. */
static void BOOT hw_put100(uint32_t v)
{
    uint32_t units = hw_div(v, 100u);
    uint8_t r = (uint8_t)(v - hw_mul(units, 100u));
    hw_putu(units);
    hw_putc('.');
    if (r < 10u) hw_putc('0');
    hw_putu(r);
}
static void BOOT hw_putx(uint16_t v)
{
    uint8_t i, d;
    for (i = 0; i < 4u; ++i, v = (uint16_t)(v << 4)) {
        d = (uint8_t)(v >> 12);
        hw_putc((char)(d < 10u ? '0' + d : 'A' - 10 + d));
    }
}
static void BOOT hw_dma(uint8_t command, volatile uint8_t *c64, uint32_t reu, uint16_t n,
                        uint8_t fix)
{
    REU[2] = (uint8_t)(uint16_t)c64;
    REU[3] = (uint8_t)((uint16_t)c64 >> 8);
    REU[4] = (uint8_t)reu;
    REU[5] = (uint8_t)(reu >> 8);
    REU[6] = (uint8_t)(reu >> 16);
    REU[7] = (uint8_t)n;
    REU[8] = (uint8_t)(n >> 8);
    REU[10] = fix;
    REU[1] = command;
    REU[10] = 0;
}
static volatile uint8_t hw_byte HW_DATA = 0;
static uint8_t BOOT hw_peek(uint32_t at)
{
    hw_dma(APOLLO_REU_FETCH, &hw_byte, at, 1u, 0);
    return hw_byte;
}
static void BOOT hw_poke(uint32_t at, uint8_t v)
{
    hw_byte = v;
    hw_dma(APOLLO_REU_STASH, &hw_byte, at, 1u, 0);
}
/* 32 KB through the REU scratch one way, as bytes a second. */
static uint32_t BOOT hw_speed(uint8_t command)
{
    uint32_t t = now();
    if (command == APOLLO_REU_STASH)
        hw_dma(command, (volatile uint8_t *)0x0801, DP_SCRATCH, 0x8000u, 0);
    else
        hw_dma(command, &hw_byte, DP_SCRATCH, 0x8000u, 0x80u); /* the C64 address held */
    t = now() - t;
    return hw_div(hw_hz << 11, t >> 4);
}
static void BOOT hw_start(void)
{
    uint16_t i;
    uint8_t top = 0, seen, keep0, keep;
    /* The text screen at $0400 with the ROM characters, white on black. */
    CIA2[0] |= 3u;
    VIC[0x11] = 0x1bu;
    VIC[0x16] = 0xc8u;
    VIC[0x18] = 0x15u;
    VIC[0x20] = 0;
    VIC[0x21] = 0;
    for (i = 0; i < 1000u; ++i) SCREEN[i] = 0x20u;
    hw_puts(t_title);
    /* PAL counts 312 raster lines, NTSC 262 or 263. */
    while (!(VIC[0x11] & 0x80u)) {
    }
    while (VIC[0x11] & 0x80u)
        if (VIC[0x12] > top) top = VIC[0x12];
    hw_hz = top >= 0x20u ? 985248ul : 1022727ul;
    hw_puts(top >= 0x20u ? t_pal : t_ntsc);
    /* The REU's size: the first bank (128 KB up) whose address wraps to 0;
     * the bytes it touches are put back. It stops at 1 MB, so that every
     * probe stays inside the store (tools/reu_layout.py): no wrap there
     * means 2 MB or more. */
    keep0 = hw_peek(0);
    for (i = 2; i <= 16u; i = (uint16_t)(i << 1)) {
        keep = hw_peek((uint32_t)i << 16);
        hw_poke(0, 0x55u);
        hw_poke((uint32_t)i << 16, 0xaau);
        seen = hw_peek(0);
        hw_poke((uint32_t)i << 16, keep);
        hw_poke(0, keep0);
        if (seen == 0xaau) break;
    }
    hw_puts(t_reu);
    hw_putu((uint32_t)i << 6);
    hw_puts(i > 16u ? t_kb_more : t_kb);
    timer_start();
    hw_puts(t_dma);
    hw_putu(hw_speed(APOLLO_REU_STASH));
    hw_puts(t_dma2);
    hw_putu(hw_speed(APOLLO_REU_FETCH));
    hw_puts(t_run);
    apollo_reu_transfer(APOLLO_REU_STASH, __rk_boot_start, DP_HW_SAVE,
                        (uint16_t)(__rk_boot_end - __rk_boot_start));
}
static uint16_t BOOT hw_crc_byte(uint16_t c, uint8_t b)
{
    uint8_t k;
    c ^= (uint16_t)b << 8;
    for (k = 0; k < 8u; ++k) c = (c & 0x8000u) ? (uint16_t)(c << 1) ^ 0x1021u : (uint16_t)(c << 1);
    return c;
}
/* CRC-16/CCITT of the end state, as tools/flight_disk.py check_crc. */
static uint16_t BOOT hw_crc(uint16_t cs)
{
    uint16_t c = 0xffffu, i, w;
    for (i = 020u; i < 2048u; ++i) {
        if (i >= 070u && i <= 073u) continue;
        w = IMAGE->words[i];
        if (i >= 037u && i <= 041u && w == 077777u) w = 0; /* a PIPA's -0 is +0 */
        c = hw_crc_byte(c, (uint8_t)w);
        c = hw_crc_byte(c, (uint8_t)(w >> 8));
    }
    c = hw_crc_byte(c, (uint8_t)cs);
    return hw_crc_byte(c, (uint8_t)(cs >> 8));
}
static void hw_report(enum exec_status st);
/* The result, once the boot code is back: the CRC and the verdict at
 * $c036-$c038, the mailbox's $a5, then the report. */
static void BOOT hw_result(enum exec_status st)
{
    hw_end_crc = hw_crc((uint16_t)(mail[0x32] | mail[0x33] << 8));
    hw_pass = st == EXEC_OK && replay_ended && hw_end_crc == DP_HW_EXPECT;
    mail[0x36] = (uint8_t)hw_end_crc;
    mail[0x37] = (uint8_t)(hw_end_crc >> 8);
    mail[0x38] = hw_pass;
    mail[0] = 0xa5;
    hw_report(st);
}
/* The window's end (interrupts off): the boot code back from the REU (its
 * region held module code during the window), then the result. main jumps
 * here from inline assembly: a C call costs main some 240 bytes of RAM. */
__attribute__((noinline, used, noreturn)) void dp_hw_end(void)
{
    apollo_reu_transfer(APOLLO_REU_FETCH, __rk_boot_start, DP_HW_SAVE,
                        (uint16_t)(__rk_boot_end - __rk_boot_start));
    hw_result((enum exec_status)sum.status);
    for (;;) {
    }
}
__attribute__((noinline)) void dp_done(void)
{
    for (;;) {
    }
}
static void BOOT hw_report(enum exec_status st)
{
    uint16_t agc = (uint16_t)(sum.end_cs - sum.start_cs) & 037777u;
    uint32_t hz100 = hw_div(hw_hz, 100u);
    hw_puts(t_agc);
    hw_putu(agc);
    hw_puts(t_cs);
    hw_putu(sum.steps);
    hw_puts(t_steps);
    hw_put100(hw_div(sum.cycles, hz100));
    hw_puts(t_s);
    if (agc) hw_put100(hw_div(sum.cycles, hw_div(hw_mul(hz100, agc), 100u)));
    hw_puts(t_x);
    hw_putx(hw_end_crc);
    hw_puts(t_vice);
    hw_putx(DP_HW_EXPECT);
    hw_puts(t_host);
    hw_putx(DP_HW_HOST);
    hw_puts(hw_end_crc == DP_HW_HOST ? t_same : t_differ);
    hw_puts(hw_pass ? t_pass : t_fail);
    if (st != EXEC_OK) {
        hw_puts(t_status);
        hw_putu((uint8_t)st);
        hw_putc(')');
    }
    dp_done();
}
#endif
int main(void){
 uint16_t job=0,task=0;
 uint32_t t,dil;
 enum exec_status st=EXEC_OK;
 if(rk_boot()){mail[1]=1;mail[5]=1;mail[0]=0xa5;for(;;){}}
#ifdef DP_HW_CHECK
 hw_start();
#endif
 load_snapshot();
 __asm__ volatile("sei");
 rk_ticks=ov2_irq_count;
 __asm__ volatile("cli");
 dil = rk_dilation;
 clock_start();
 sum.start_cs = agc_cs();
#ifdef DP_SWEEP
 sweep_next(0u, 0u);
#endif
 job=rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];task=rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];
 timer_start();
 t=now();
 for(;;){uint32_t s0=now(),s1;
#ifdef DP_STEPS
  uint16_t z0=IMAGE->words[5],fb0=IMAGE->words[4],sb0=rk_device.superbank;
#ifdef DP_REPLAY
  /* With the replay, cycles holds the clock at the step's start instead:
   * TIME1's low byte, rk_tick_phase and rk_work (host FLIGHT_REPLAY_TRACE). */
  uint32_t clk0 =
      ((uint32_t)(uint8_t)IMAGE->words[025] << 24) | ((uint32_t)rk_tick_phase << 16) | rk_work;
#endif
#endif
  rk_rupt_last=0;
#ifdef DP_REPLAY
  if (replay_due()) {
      replay_ended = 1;
#ifdef DP_SWEEP
      if (sweep_next(1u, EXEC_OK)) continue;
#else
      if (r.steps) close_record(s0 - t, EXEC_OK);
#endif
      break;
  }
#endif
#ifdef DP_KEYS
  press_due(window_cs());
#endif
#ifdef RK_STEP_PROFILE
  prof_last=now();
#endif
  st=rk_resident_step(AGC_TRANSFER_NONE);
#ifdef RK_STEP_PROFILE
  rk_prof(10);
#endif
  s1=now();
  ++sum.steps;
  /* A record ends where the step that takes the next interrupt starts (one
   * interrupt can end and the next start between two steps, or one can
   * start and end in one step), and where an interrupt goes off. */
#ifdef DP_STEPS
  /* One record per step (debugging): the Z (steps) and physical bank
   * (job_loads: FB, or 040-047 for FB 030-037 with superbank bit 0100) it
   * started at, and in task_loads 1 if the job slot changed, 2 the task slot. */
  {r.request=rk_rupt_last;r.steps=z0;r.task_loads=0;
   r.job_loads=(uint8_t)((fb0>>10)+((fb0>=060000u&&(sb0&0100u))?010u:0u));
   if(rk_runtime.loaded[APOLLO_OV2_SLOT_JOB]!=job)r.task_loads|=1;
   if(rk_runtime.loaded[APOLLO_OV2_SLOT_TASK]!=task)r.task_loads|=2;
   job=rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];task=rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];
#ifdef DP_REPLAY
   close_record(clk0, st);
#else
   close_record(s1-s0,st);
#endif
   if (st != EXEC_OK || window_cs() >= DP_CS) break;
   continue;}
#endif
#ifdef DP_SWEEP
  /* The sweep keeps no interrupt records: a case ends at a step error or
   * DP_CS centiseconds in, and the next one starts. */
  (void)s0;
  (void)s1;
  (void)job;
  (void)task;
  (void)t;
  if (st != EXEC_OK || ((uint16_t)(agc_cs() - sum.start_cs) & 037777u) >= DP_CS) {
      if (sweep_next(0u, (uint8_t)st)) {
          st = EXEC_OK;
          continue;
      }
      break;
  }
 }
#else
  if(rk_rupt_last){if(r.steps)close_record(s0-t,EXEC_OK),t=s0;r.request=rk_rupt_last;}
  ++r.steps;
  if(rk_runtime.loaded[APOLLO_OV2_SLOT_JOB]!=job){job=rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];if(job)++r.job_loads;}
  if(rk_runtime.loaded[APOLLO_OV2_SLOT_TASK]!=task){task=rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];if(task)++r.task_loads;}
  if (st != EXEC_OK || window_cs() >= DP_CS) {
      close_record(s1 - t, st);
      break;
  }
  if(r.request&&!rk_hardware.in_interrupt){close_record(s1-t,EXEC_OK);t=s1;}}
#endif
 sum.cycles=now();
 __asm__ volatile("sei");
 sum.dilation=rk_dilation-dil;sum.end_cs=agc_cs();sum.status=(uint8_t)st;
 sum.z=IMAGE->words[5];sum.fb=IMAGE->words[4];sum.superbank=rk_device.superbank;
#ifdef DP_REPLAY
 {
     uint16_t t = replay_cs();
     {
         uint16_t n = DP_REPLAY_ROWS - rows_left;
         mail[0x30] = (uint8_t)n;
         mail[0x31] = (uint8_t)(n >> 8);
     }
     mail[0x32] = (uint8_t)t;
     mail[0x33] = (uint8_t)(t >> 8);
     mail[0x34] = replay_ended;
     mail[0x35] = states;
 }
#endif
#ifndef DP_SWEEP
 /* (The sweep keeps no records; a 0-byte REU transfer would move 64 KB.) */
 apollo_reu_transfer(APOLLO_REU_FETCH, RECORDS, DP_SCRATCH,
                     (sum.records < DP_KEEP ? sum.records : DP_KEEP) * sizeof r);
#endif
 *SUMMARY=sum;
#ifdef RK_STEP_PROFILE
 memcpy(PROFILE, prof, DP_SEGMENTS * sizeof(dp_segment));
#endif
#ifdef DP_WATCH
 /* Up to five erasable words at the end of the window (DP_WATCH: their
  * octal addresses, comma separated), at $c006 for the dump. */
 {
     static const uint16_t w[] = {DP_WATCH};
     uint8_t i;
     for (i = 0; i < sizeof w / sizeof w[0] && i < 5u; ++i) {
         mail[6u + 2u * i] = (uint8_t)IMAGE->words[w[i]];
         mail[7u + 2u * i] = (uint8_t)(IMAGE->words[w[i]] >> 8);
     }
 }
#endif
 mail[1]=(uint8_t)(st!=EXEC_OK);mail[2]=0;mail[3]=(uint8_t)sum.records;mail[4]=(uint8_t)(sum.records>>8);
 mail[5]=(uint8_t)(st!=EXEC_OK?2:0);
#ifdef DP_HW_CHECK
 __asm__ volatile("jmp dp_hw_end");
#endif
 mail[0]=0xa5;
 for(;;){}
}
