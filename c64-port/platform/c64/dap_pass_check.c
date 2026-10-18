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
 * $c005 the first failing step; $c010 the summary (dp_summary); $a5 at
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
static dp_segment prof[DP_SEGMENTS];
static uint32_t prof_last;
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
 * without stopping them (B again if A wrapped between the reads). */
#define CIA2 ((volatile uint8_t *)0xdd00)
static void BOOT timer_start(void){
 CIA2[0x0e]=0;CIA2[0x0f]=0;
 CIA2[0x04]=0xff;CIA2[0x05]=0xff;CIA2[0x06]=0xff;CIA2[0x07]=0xff;
 CIA2[0x0f]=0x51;
 CIA2[0x0e]=0x11;}
static uint32_t now(void){uint8_t b0,b1,a0,a1;
 do{b1=CIA2[0x07];b0=CIA2[0x06];a1=CIA2[0x05];a0=CIA2[0x04];}while(b0!=CIA2[0x06]);
 return ~(((uint32_t)b1<<24)|((uint32_t)b0<<16)|((uint16_t)a1<<8)|a0);}
#ifdef RK_STEP_PROFILE
void rk_prof(uint8_t segment){uint32_t t=now();prof[segment].cycles+=t-prof_last;++prof[segment].calls;prof_last=now();}
#endif
static uint16_t agc_cs(void){return (uint16_t)IMAGE->words[025];}

static void BOOT load_snapshot(void){
 memcpy(IMAGE->words,snap_words,sizeof snap_words);
 SNAP_IO(rk_device)
 SNAP_HW(rk_hardware)
 SNAP_FLIGHT(rk_flight)
 rk_pending=0;}

static dp_record r;
static dp_summary sum;
static void close_record(uint32_t cycles,enum exec_status st){
 r.cycles=cycles;r.status=(uint8_t)st;
 if(sum.records<DP_MAX){
  apollo_reu_transfer(APOLLO_REU_STASH,&r,DP_SCRATCH+(uint32_t)sum.records*sizeof r,sizeof r);
  ++sum.records;}
 else sum.lost=1;
 r.steps=0;r.job_loads=0;r.task_loads=0;r.request=0;}

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
#endif
  rk_rupt_last=0;
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
   close_record(s1-s0,st);
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
 apollo_reu_transfer(APOLLO_REU_FETCH, RECORDS, DP_SCRATCH,
                     (sum.records < DP_KEEP ? sum.records : DP_KEEP) * sizeof r);
 *SUMMARY=sum;
#ifdef RK_STEP_PROFILE
 memcpy(PROFILE,prof,sizeof prof);
#endif
 mail[1]=(uint8_t)(st!=EXEC_OK);mail[2]=0;mail[3]=(uint8_t)sum.records;mail[4]=(uint8_t)(sum.records>>8);
 mail[5]=(uint8_t)(st!=EXEC_OK?2:0);
 mail[0]=0xa5;
 for(;;){}
}
