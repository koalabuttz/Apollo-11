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
 *     are timed in CPU cycles with CIA2's cascaded timers, interrupts off;
 *  3. once a module is loaded, runs one of its resolver records' points
 *     twice from the same state (the boot image and the module's slot, kept
 *     in the REU at FL_SCRATCH): through the module's main (ABI 2 run) and
 *     through the resident's direct call (rk_direct), with the module's
 *     slot the only loaded one. Status, image and channels
 *     (agc_hardware) must match; both are timed.
 *  0. before any of that, with a pinned module (slot class 3, FL_PINNED):
 *     one of its points through the module packing the same table and
 *     through rk_direct with only the pinned module present (steps 10-11);
 *     the pinned module is then dropped, since the mailbox reuses its RAM.
 *
 * FL_DATA names the generated data header (records, modules, FL_IDLE_Z).
 * For bisecting a crash, -DFL_FIRST=n -DFL_LIMIT=n run modules [FIRST,LIMIT)
 * only and -DFL_NO_PROFILE skips profile(); $c006 holds the module index
 * being run; -DFL_PROFILE_MARKS stores to $c007 and $c008 around each cached
 * run, for a VICE monitor watch that profiles it. Mailbox (tools/flight_vice_load.py): $c001-2 failures, $c003-4 checks,
 * $a5 at $c000; $c005 the first failing step; $c3f8 the timer's own cost
 * (4 bytes), $c3f0-5 the call profile (profile()), $c3f6 the soft-stack peak, $c3fe the module count, $c400 one fl_timing per module (up to
 * $cbff, FL_MODULES at most 204), $c3e0 the indices of up to 8 modules whose
 * direct call differs from their main ($c3df their count). */
#include <stdint.h>
#include "resident_flight.h"
#define BOOT_DATA __attribute__((section(".boot.text.fl_records")))
#define BOOT __attribute__((noinline,section(".boot.text.fl_check")))
typedef struct { uint16_t bank, loc, module; } fl_record;
typedef struct { uint16_t id, bytes, bank, loc; } fl_module;   /* loc 0: no record to call */
/* In fl_modules order (tools/flight_vice_load.py reads ids and sizes from
 * the FL_DATA header). */
typedef struct { uint32_t load; uint16_t cached, direct, main; } fl_timing;
#include FL_DATA
#ifndef FL_FIRST
#define FL_FIRST 0u
#endif
#ifndef FL_LIMIT
#define FL_LIMIT FL_MODULES
#endif
/* The timings at $C400 leave the soft stack (below $D000) 1 KB. */
_Static_assert(0xc400ul+FL_MODULES*sizeof(fl_timing)<=0xcc00ul,"timings would reach the stack");
#define FL_IMAGE_BYTES 4096ul
_Static_assert(sizeof(apollo_overlay_state)<=FL_IMAGE_BYTES,"the image snapshot overruns its scratch");
_Static_assert(FL_IMAGE_BYTES+16384u<=FL_SCRATCH_BYTES,"a slot snapshot overruns the REU scratch");
/* fl_modules is boot data (the first module load overwrites it): boot
 * stashes it in the REU scratch after the slot snapshot, and the loop
 * fetches one entry at a time into module. */
#define FL_MODULE_TABLE (FL_SCRATCH+FL_IMAGE_BYTES+16384ul)
_Static_assert(FL_IMAGE_BYTES+16384ul+sizeof fl_modules<=FL_SCRATCH_BYTES-256u,"the module table overruns the REU scratch");
static fl_module module;
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;
static fl_timing *const timing=(fl_timing *)0xc400;
static uint16_t failures,checks;
static uint8_t first_failure;   /* to $c005 at the end: the mailbox page is the pinned module's until then */
static void expect(uint8_t ok,uint8_t step){++checks;if(!ok){++failures;if(!first_failure)first_failure=step;}}

static void BOOT resolve_records(void){uint16_t i;apollo_ov2_cont at;
 for(i=0;i<FL_RECORDS;++i)
  expect(rk_runtime.store.resolve(rk_runtime.store.user,fl_records[i].bank,fl_records[i].loc,&at)&&
         at.module==fl_records[i].module&&at.selector==0u,2);}

/* The $D000 extension path (slot class 2): a pattern stashed in the REU
 * scratch, fetched to $D000 and verified with I/O banked out (rk_hidden_dma),
 * read back with I/O out, then CIA1 interrupts left on with I/O out: the IRQ
 * entry must restore I/O to acknowledge them, or the CPU never returns. */
extern volatile uint16_t ov2_irq_count;
static uint16_t BOOT irq_count(void){uint16_t n;do n=ov2_irq_count;while(n!=ov2_irq_count);return n;}
static void BOOT fixed_extension(void){uint8_t b[256];uint16_t i,bad=0,t;   /* on the stack: $C000 holds the pinned module */
 for(i=0;i<256u;++i)b[i]=(uint8_t)(i^0x5au);
 apollo_reu_transfer(APOLLO_REU_STASH,b,FL_SCRATCH+FL_SCRATCH_BYTES-256u,256u);
 expect(rk_hidden_dma(APOLLO_REU_FETCH,(void *)APOLLO_OV2_FIXED_ADDRESS,FL_SCRATCH+FL_SCRATCH_BYTES-256u,256u),8);
 expect(rk_hidden_dma(APOLLO_REU_VERIFY,(void *)APOLLO_OV2_FIXED_ADDRESS,FL_SCRATCH+FL_SCRATCH_BYTES-256u,256u),8);
 __asm__ volatile("sei");
 *(volatile uint8_t *)1=0x34;
 for(i=0;i<256u;++i)bad|=(uint8_t)(((volatile uint8_t *)APOLLO_OV2_FIXED_ADDRESS)[i]^(uint8_t)(i^0x5au));
 t=irq_count();
 __asm__ volatile("cli");
 for(i=0;i<60000u&&(uint16_t)(irq_count()-t)<3u;++i){}
 __asm__ volatile("sei");
 *(volatile uint8_t *)1=0x35;
 __asm__ volatile("cli");
 expect(!bad,8);
 expect((uint16_t)(irq_count()-t)>=3u,9);}

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

/* Where a cached call goes: the runtime alone (a cached run of the
 * boundary-word module, whose main only returns), and the resident's own
 * transfer point and resolve from FL_IDLE_Z, which every flight module
 * repeats with its own copy. Cycles, timer cost included, at $c3f0-$c3f5. */
static void profile(void){apollo_ov2_cont at={RK_MODULE_BOUNDARY,0},resume;uint16_t value;uint8_t action;
 volatile uint16_t *out=(volatile uint16_t *)0xc3f0;
 IMAGE->words[5]=FL_IDLE_Z;IMAGE->words[4]=0;
 apollo_ov2_run(&rk_runtime,0,at,&value,&action,&resume);
 timer_start();apollo_ov2_run(&rk_runtime,0,at,&value,&action,&resume);out[0]=(uint16_t)timer_stop();
 timer_start();(void)agc_transfer_point(&rk_program,IMAGE,rk_device.superbank);out[1]=(uint16_t)timer_stop();
 timer_start();(void)agc_transfer_resolve(&rk_program,IMAGE,rk_device.superbank);out[2]=(uint16_t)timer_stop();}

/* A 16-bit sum of the image and the channels (rk_hardware), rotated per byte so moved
 * words differ. */
static uint16_t sum_bytes(uint16_t h,const void *p,uint16_t n){const uint8_t *b=p;
 while(n--)h=(uint16_t)((h<<1|h>>15)+*b++);
 return h;}
static uint16_t image_sum(void){
 return sum_bytes(sum_bytes(0,IMAGE,sizeof *IMAGE),&rk_hardware,sizeof rk_hardware);}
static rk_io device;
static agc_hardware hardware;
static void direct_state(const fl_module *m){
 apollo_reu_transfer(APOLLO_REU_FETCH,IMAGE,FL_SCRATCH,sizeof *IMAGE);
 rk_device=device;rk_hardware=hardware;
 IMAGE->words[4]=(agc_word)(m->bank&076000u);IMAGE->words[5]=m->loc;
 rk_device.superbank=(agc_word)(m->bank&0100u);}
static uint8_t differ;
static uint16_t direct(uint16_t k,const fl_module *m,uint16_t *main_cycles){apollo_ov2_cont at={m->id,0},resume;uint16_t value,sum,t;
 uint8_t ok;
 uint8_t action,i;enum exec_status a,b;
 uint8_t *slot=0;uint16_t bytes=0;
 for(i=0;i<APOLLO_OV2_SLOTS;++i)if(rk_runtime.loaded[i]==m->id){slot=rk_runtime.slot[i];bytes=apollo_ov2_slot_bytes(i);}
 expect(slot!=0,7);
 if(!slot)return 0;
 /* The whole slot too: a module's own statics (device records, step state)
  * must start both runs alike. */
 apollo_reu_transfer(APOLLO_REU_STASH,slot,FL_SCRATCH+FL_IMAGE_BYTES,bytes);
 direct_state(m);value=(uint16_t)(uintptr_t)&rk_env;rk_env.status=EXEC_OK;
 timer_start();ok=apollo_ov2_run(&rk_runtime,0,at,&value,&action,&resume)==APOLLO_OV2_OK;*main_cycles=(uint16_t)timer_stop();
 expect(ok,5);
 a=rk_env.status;sum=image_sum();
 /* Tables on another io record run only through the main (no direct call
  * to compare). */
 {const rk_module_tables *tb=(const rk_module_tables *)(uintptr_t)(slot[APOLLO_OV2_H_TABLES]|(uint16_t)slot[APOLLO_OV2_H_TABLES+1]<<8);
  if(tb&&tb->ready==RK_TABLES_NOT_DIRECT){
   apollo_reu_transfer(APOLLO_REU_FETCH,IMAGE,FL_SCRATCH,sizeof *IMAGE);rk_device=device;rk_hardware=hardware;
   return 0;}}
 for(i=0;i<APOLLO_OV2_SLOTS;++i)if(rk_runtime.loaded[i]!=m->id)rk_runtime.loaded[i]=0;
 direct_state(m);
 apollo_reu_transfer(APOLLO_REU_FETCH,slot,FL_SCRATCH+FL_IMAGE_BYTES,bytes);
 timer_start();b=rk_direct(AGC_TRANSFER_NONE);t=(uint16_t)timer_stop();
 ok=a==b&&image_sum()==sum;expect(ok,6);
 if(!ok&&differ<8u){((volatile uint16_t *)0xc3e0)[differ++]=k;mail[0x3df]=differ;}
 apollo_reu_transfer(APOLLO_REU_FETCH,IMAGE,FL_SCRATCH,sizeof *IMAGE);rk_device=device;rk_hardware=hardware;
 return t;}

#ifdef FL_PINNED
/* The pinned module (slot class 3): boot loaded it at $C000 and built its
 * tables (rk_pinned). One of its points, from the boot state, through the
 * module that packs the same table (its main, ABI 2 run) and then through
 * rk_direct with no other module loaded: status, image and channels must
 * match. Then it is dropped (rk_pinned 0): the mailbox and the timings
 * reuse $C000-$CBFF. */
static void pinned(void){static const fl_module p={FL_PINNED_MODULE,0,FL_PINNED_BANK,FL_PINNED_LOC};
 apollo_ov2_cont at={FL_PINNED_MODULE,0},resume;uint16_t value,sum;uint8_t action,i;enum exec_status a;
 expect(rk_pinned&&((const uint8_t *)APOLLO_OV2_PINNED_ADDRESS)[APOLLO_OV2_H_ID]==(uint8_t)FL_PINNED,10);
 direct_state(&p);value=(uint16_t)(uintptr_t)&rk_env;rk_env.status=EXEC_OK;
 expect(apollo_ov2_run(&rk_runtime,0,at,&value,&action,&resume)==APOLLO_OV2_OK,11);
 a=rk_env.status;sum=image_sum();
 for(i=0;i<APOLLO_OV2_SLOTS;++i)rk_runtime.loaded[i]=0;
 direct_state(&p);
 expect(rk_pinned&&rk_direct(AGC_TRANSFER_NONE)==a&&image_sum()==sum,11);
 rk_pinned=0;
 apollo_reu_transfer(APOLLO_REU_FETCH,IMAGE,FL_SCRATCH,sizeof *IMAGE);rk_device=device;rk_hardware=hardware;}
#endif

/* Soft-stack watermark: free stack below the timings is filled before the
 * module runs and scanned after; $c3f6 gets the peak depth below $D000. */
#define FL_STACK_FLOOR ((volatile uint8_t *)(0xc400u+FL_MODULES*sizeof(fl_timing)))
static void stack_fill(void){volatile uint8_t here,*p=FL_STACK_FLOOR;
 while(p<&here-16)*p++=0x5a;}
static void stack_peak(void){volatile uint8_t *p=FL_STACK_FLOOR;
 while(*p==0x5a)++p;
 *(volatile uint16_t *)0xc3f6=(uint16_t)(0xd000u-(uintptr_t)p);}

int main(void){uint16_t i;
 expect(rk_boot()==0,1);
 if(!failures){
  apollo_reu_transfer(APOLLO_REU_STASH,(void *)fl_modules,FL_MODULE_TABLE,sizeof fl_modules);
  resolve_records();
  fixed_extension();
  __asm__ volatile("sei");
  apollo_reu_transfer(APOLLO_REU_STASH,IMAGE,FL_SCRATCH,sizeof *IMAGE);device=rk_device;hardware=rk_hardware;
#ifdef FL_PINNED
  pinned();
#endif
  rk_pinned=0;
  for(i=1;i<0x3e0u;++i)mail[i]=0;
  timer_start();*(volatile uint32_t *)0xc3f8=timer_stop();
  stack_fill();
#ifndef FL_NO_PROFILE
  profile();
#endif
  for(i=FL_FIRST;i<FL_LIMIT;++i){
   *(volatile uint16_t *)0xc006=i;
   apollo_reu_transfer(APOLLO_REU_FETCH,&module,FL_MODULE_TABLE+i*sizeof module,sizeof module);
   timing[i].load=run(module.id,3);
#ifdef FL_PROFILE_MARKS
   mail[7]=1;   /* a monitor watch here can start VICE's profiler */
#endif
   timing[i].cached=(uint16_t)run(module.id,4);
#ifdef FL_PROFILE_MARKS
   mail[8]=1;
#endif
   timing[i].direct=module.loc?direct(i,&module,&timing[i].main):0u;
  }
  *(volatile uint16_t *)0xc3fe=FL_LIMIT;
  stack_peak();
  __asm__ volatile("cli");}
 else for(i=1;i<0x3e0u;++i)mail[i]=0;
 mail[5]=first_failure;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);
 mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);
 mail[0]=0xa5;
 for(;;){}
}
