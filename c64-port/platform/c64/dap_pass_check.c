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
#define SNAP_DATA __attribute__((section(".boot.text.dp_snap")))
#define BOOT __attribute__((noinline,section(".boot.text.dp_check")))
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
static uint16_t agc_cs(void){return (uint16_t)IMAGE->words[025];}
#ifdef DP_KEYS
/* The crew's DSKY keys in the window (DP_KEYS, written by
 * tools/flight_scenario_vice.py --keys from the scenario's key string): key
 * k is pressed at the first step at or after dp_key_cs[k] centiseconds into
 * the window, as tools/flight_host_run.c presses it: channel 15 and a
 * KEYRUPT1 request, or for a mark key (bit 0400) channel 16 and KEYRUPT2.
 * The key stays in its channel until the next one, as on the host. */
#include "resident_rupt.h"
#include DP_KEYS
static uint8_t key_next;
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
 * --replay). DP_REPLAY_ROWS 8-byte rows at DP_REPLAY_REU in the REU, outside
 * the flight store and the resident: the centiseconds since the snapshot,
 * kind, where (0 at a sample point, 1 at any step boundary), address and
 * value, the address a C64 one (tools/flight_scenario_vice.py c64_rows
 * turns the host's erasable words, channels and requests into the
 * addresses of the image, rk_hardware, rk_flight, rk_device's copies and
 * rk_pending). Kinds: 1 a signed count added to a counter word (a PIPA),
 * 3 bits ORed in (interrupt requests), 4 a byte stored, else a word
 * stored. A row is applied before the first step at or after its time where
 * it may be (rows go in order: a sample point's row holds back the
 * boundary rows after it). The sample point is the host's: Z at ADVAN in
 * bank 01, NEWJOB -0, no interrupt running or requested; with DP_BUSY also
 * a step boundary outside an interrupt with none requested DP_BUSY
 * centiseconds after the last (FLIGHT_BUSY_SAMPLE). At the first sample
 * point DP_REPLAY_CS or more centiseconds in, before that point's rows,
 * the window ends: the centiseconds, the erasable words, rk_hardware and
 * rk_flight (RK_HARDWARE up to RK_RESTART_LAMP) and 32 bytes of rk_device
 * go to DP_END in the REU, after the same state at the first sample point at or
 * after each DP_EVERY centiseconds before; the rows applied to $c030, the
 * end's centiseconds to $c032, $c034 1 if the end was reached and $c035
 * the states written. */
#include "resident_rupt.h"
#if RK_LM
#define DP_ADVAN 03214u
#else
#define DP_ADVAN 03231u
#endif
typedef struct { uint16_t cs; uint8_t kind, where; uint16_t addr, value; } __attribute__((packed)) dp_row;
_Static_assert(sizeof(dp_row)==8u,"rows are 8 bytes (tools/flight_host_run.c rp_row)");
/* The row being applied sits in the cassette buffer ($03F4-$03FB; the step
 * profile uses its bottom), not in the resident's RAM. */
#define row (*(dp_row *)0x03f4u)
static uint16_t rows_left = DP_REPLAY_ROWS;
static uint8_t row_ready;
static uint32_t row_at = DP_REPLAY_REU, state_at = DP_END;
static uint16_t replay_t0;
#ifdef DP_BUSY
static uint16_t busy_last;
#endif
#ifndef DP_EVERY
#define DP_EVERY DP_REPLAY_CS
#endif
#define DP_IO_BYTES (RK_RESTART_LAMP - RK_HARDWARE)
static uint16_t state_next = DP_EVERY;
static uint8_t states;
/* TIME1 since the snapshot: a window is at most 16,383 cs. */
static uint16_t replay_cs(void){return (uint16_t)(IMAGE->words[025]-replay_t0)&037777u;}
static uint8_t sample_point(uint16_t t){
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
__attribute__((noinline)) static void stash(void *from, uint16_t n){
    apollo_reu_transfer(APOLLO_REU_STASH, from, state_at, n);
    state_at += n;
}
/* The state at a sample point: its centiseconds, the erasable words,
 * rk_hardware and rk_flight, then 32 bytes of rk_device. */
__attribute__((noinline)) static void replay_state(uint16_t t){
    static uint16_t at;
    at = t;
    ++states;
    stash(&at, 2u);
    stash(IMAGE->words, 2048u * sizeof(agc_word));
    stash((void *)RK_HARDWARE, DP_IO_BYTES);
    stash(&rk_device, 32u);
}
/* The rows due before this step; 1 when the window ends here. */
__attribute__((noinline)) static uint8_t replay_due(void){
    uint16_t t = replay_cs();
    uint8_t sp = sample_point(t);
    if (sp) {
#ifdef DP_BUSY
        busy_last = t;
#endif
        if (t >= DP_REPLAY_CS) {
            replay_state(t);
            return 1;
        }
        if (t >= state_next) {
            replay_state(t);
            while (state_next <= t) state_next += DP_EVERY;
        }
    }
    while (rows_left) {
        uint16_t *w;
        if (!row_ready) {
            apollo_reu_transfer(APOLLO_REU_FETCH, &row, row_at, sizeof(dp_row));
            row_at += sizeof(dp_row);
            row_ready = 1;
        }
        if (row.cs > t || (!row.where && !sp)) return 0;
        w = (uint16_t *)row.addr;
        __asm__ volatile("sei");
        if (row.kind == 1) {
            /* tools/flight_host_run.c pipa_add: a signed count into a ones'
             * complement counter, held at +-037777. */
            int16_t v = (int16_t)((*w & 040000u) ? -(int16_t)((~*w) & 037777u) : (int16_t)*w) + (int16_t)row.value;
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
    return 0;
}
#endif

static void BOOT load_snapshot(void){
 memcpy(IMAGE->words,snap_words,sizeof snap_words);
#ifdef RK_STEP_PROFILE
 memset(prof, 0, DP_SEGMENTS * sizeof(dp_segment));
#endif
 SNAP_IO(rk_device)
 SNAP_HW(rk_hardware)
 SNAP_FLIGHT(rk_flight)
 rk_pending=0;
#ifdef DP_REPLAY
 /* The host's replay runs on the work clock from its first step. */
 rk_work=0;rk_tick_phase=0;
 replay_t0=IMAGE->words[025];
#endif
}

static dp_record r;
static dp_summary sum;
static void close_record(uint32_t cycles,enum exec_status st){
 r.cycles=cycles;r.status=(uint8_t)st;
 if(sum.records<DP_MAX){
  apollo_reu_transfer(APOLLO_REU_STASH,&r,DP_SCRATCH+(uint32_t)sum.records*sizeof r,sizeof r);
  ++sum.records;}
 else sum.lost=1;
 r.steps=0;r.job_loads=0;r.task_loads=0;r.request=0;}

#ifdef DP_REPLAY
static uint8_t replay_ended;
#endif
int main(void){
 uint16_t job=0,task=0;
 uint32_t t,dil;
 enum exec_status st=EXEC_OK;
 if(rk_boot()){mail[1]=1;mail[5]=1;mail[0]=0xa5;for(;;){}}
 load_snapshot();
 __asm__ volatile("sei");
 rk_ticks=ov2_irq_count;
 __asm__ volatile("cli");
 dil=rk_dilation;sum.start_cs=agc_cs();
 job=rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];task=rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];
 timer_start();
 t=now();
 for(;;){uint32_t s0=now(),s1;
#ifdef DP_STEPS
  uint16_t z0=IMAGE->words[5],fb0=IMAGE->words[4],sb0=rk_device.superbank;
#ifdef DP_REPLAY
  /* With the replay, cycles holds the clock at the step's start instead:
   * TIME1's low byte, rk_tick_phase and rk_work (host FLIGHT_REPLAY_TRACE). */
  uint32_t clk0=((uint32_t)(uint8_t)IMAGE->words[025]<<24)|((uint32_t)rk_tick_phase<<16)|rk_work;
#endif
#endif
  rk_rupt_last=0;
#ifdef DP_REPLAY
  if(replay_due()){replay_ended=1;if(r.steps)close_record(s0-t,EXEC_OK);break;}
#endif
#ifdef DP_KEYS
  press_due((uint16_t)(agc_cs() - sum.start_cs) & 037777u);
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
   close_record(clk0,st);
#else
   close_record(s1-s0,st);
#endif
   if(st!=EXEC_OK||((uint16_t)(agc_cs()-sum.start_cs)&037777u)>=DP_CS)break;
   continue;}
#endif
  if(rk_rupt_last){if(r.steps)close_record(s0-t,EXEC_OK),t=s0;r.request=rk_rupt_last;}
  ++r.steps;
  if(rk_runtime.loaded[APOLLO_OV2_SLOT_JOB]!=job){job=rk_runtime.loaded[APOLLO_OV2_SLOT_JOB];if(job)++r.job_loads;}
  if(rk_runtime.loaded[APOLLO_OV2_SLOT_TASK]!=task){task=rk_runtime.loaded[APOLLO_OV2_SLOT_TASK];if(task)++r.task_loads;}
  if(st!=EXEC_OK||((uint16_t)(agc_cs()-sum.start_cs)&037777u)>=DP_CS){close_record(s1-t,st);break;}
  if(r.request&&!rk_hardware.in_interrupt){close_record(s1-t,EXEC_OK);t=s1;}}
 sum.cycles=now();
 __asm__ volatile("sei");
 sum.dilation=rk_dilation-dil;sum.end_cs=agc_cs();sum.status=(uint8_t)st;
 sum.z=IMAGE->words[5];sum.fb=IMAGE->words[4];sum.superbank=rk_device.superbank;
#ifdef DP_REPLAY
 {uint16_t t=replay_cs();{uint16_t n=DP_REPLAY_ROWS-rows_left;mail[0x30]=(uint8_t)n;mail[0x31]=(uint8_t)(n>>8);}mail[0x32]=(uint8_t)t;mail[0x33]=(uint8_t)(t>>8);mail[0x34]=replay_ended;mail[0x35]=states;}
#endif
 apollo_reu_transfer(APOLLO_REU_FETCH, RECORDS, DP_SCRATCH,
                     (sum.records < DP_KEEP ? sum.records : DP_KEEP) * sizeof r);
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
 mail[0]=0xa5;
 for(;;){}
}
