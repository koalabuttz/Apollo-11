/* SPDX-License-Identifier: MIT */
/* flight-store-vice: the flight resident booted on the whole-program flight
 * store (tools/flight_store_build.py, docs/FLIGHT-STORE.md). The program
 * boots like the flight resident (KERNAL out, the store opened and
 * verified, the boundary-word module run), then
 *
 *  1. resolves every resolver record of the store through the resident's
 *     resolver and checks the module it names (before any module load: the
 *     records live in the boot-only code the first load overwrites);
 *  2. runs every flight module twice from a point no table holds (FL_IDLE_Z):
 *     the first run loads it from the REU into its slot, the second finds it
 *     loaded. Each run must return to the resident with EXEC_UNMAPPED. Both
 *     are timed in CPU cycles with CIA2's cascaded timers, interrupts off.
 *
 * FL_DATA names the generated data header (records, modules, FL_IDLE_Z).
 * Mailbox (tools/flight_vice_load.py): $c001-2 failures, $c003-4 checks,
 * $a5 at $c000; $c005 the first failing step; $c3f8 the timer's own cost
 * (4 bytes), $c3fe the module count, $c400 one fl_timing per module. */
#include <stdint.h>
#include "resident_flight.h"
#define BOOT_DATA __attribute__((section(".boot.text.fl_records")))
#define BOOT __attribute__((noinline,section(".boot.text.fl_check")))
typedef struct { uint16_t bank, loc, module; } fl_record;
typedef struct { uint16_t id, bytes; } fl_module;
typedef struct { uint16_t id, bytes; uint32_t load, cached; } fl_timing;
#include FL_DATA
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;
static fl_timing *const timing=(fl_timing *)0xc400;
static uint16_t failures,checks;
static void expect(uint8_t ok,uint8_t step){++checks;if(!ok){++failures;if(!mail[5])mail[5]=step;}}

static void BOOT resolve_records(void){uint16_t i;apollo_ov2_cont at;
 for(i=0;i<FL_RECORDS;++i)
  expect(rk_runtime.store.resolve(rk_runtime.store.user,fl_records[i].bank,fl_records[i].loc,&at)&&
         at.module==fl_records[i].module&&at.selector==0u,2);}

/* CIA2 timer A counts cycles from $FFFF, timer B its underflows. */
#define CIA2 ((volatile uint8_t *)0xdd00)
static void timer_start(void){
 CIA2[0x0e]=0;CIA2[0x0f]=0;
 CIA2[0x04]=0xff;CIA2[0x05]=0xff;CIA2[0x06]=0xff;CIA2[0x07]=0xff;
 CIA2[0x0f]=0x51;   /* B: force load, count A underflows, start */
 CIA2[0x0e]=0x11;}  /* A: force load, count cycles, start */
static uint32_t timer_stop(void){uint16_t a,b;
 CIA2[0x0e]=0;CIA2[0x0f]=0;
 a=(uint16_t)(CIA2[0x04]|CIA2[0x05]<<8);b=(uint16_t)(CIA2[0x06]|CIA2[0x07]<<8);
 return ((uint32_t)(uint16_t)~b<<16)|(uint16_t)~a;}

static uint32_t run(uint16_t id,uint8_t step){apollo_ov2_cont at={id,0},resume;uint16_t value;uint8_t action;
 enum apollo_ov2_status status;uint32_t t;
 IMAGE->words[5]=FL_IDLE_Z;IMAGE->words[4]=0;
 value=(uint16_t)(uintptr_t)&rk_env;rk_env.status=EXEC_OK;
 timer_start();
 status=apollo_ov2_run(&rk_runtime,0,at,&value,&action,&resume);
 t=timer_stop();
 expect(status==APOLLO_OV2_OK&&action==APOLLO_OV2_DONE&&rk_env.status==EXEC_UNMAPPED,step);
 return t;}

int main(void){uint16_t i;
 for(i=1;i<5;++i)mail[i]=0;
 mail[5]=0;
 expect(rk_boot()==0,1);
 if(!failures){
  resolve_records();
  __asm__ volatile("sei");
  timer_start();*(volatile uint32_t *)0xc3f8=timer_stop();
  for(i=0;i<FL_MODULES;++i){
   timing[i].id=fl_modules[i].id;timing[i].bytes=fl_modules[i].bytes;
   timing[i].load=run(fl_modules[i].id,3);
   timing[i].cached=run(fl_modules[i].id,4);}
  *(volatile uint16_t *)0xc3fe=FL_MODULES;
  __asm__ volatile("cli");}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);
 mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);
 mail[0]=0xa5;
 for(;;){}
}
