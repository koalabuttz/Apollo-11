/* SPDX-License-Identifier: MIT */
/* interp-flight: flight-scenario interpretive code through the flight
 * resident and the compact interpreter ($0A01 in the job slot, $0A02 at
 * $D000), from the whole-program flight store (tools/flight_store_build.py,
 * docs/INTERPRETER-RUNTIME.md). The program boots like the flight resident,
 * then for each case (tools/interp_flight_capture.c: a yaAGC flight scenario
 * at an arrival at one of the interpreter's entry points) it builds the
 * case's erasable image at $7000 and runs resident steps (rk_resident_step,
 * so the store's resolver, the module loader and the trampoline all take
 * part) while Z stays at an interpreter entry point. The exit Z, FB, a digest
 * of the image and the superbank, the step count and the last status are
 * the case's result; tools/interp_flight_test.py has yaAGC reach the same
 * state from the case (tools/interp_flight_capture.c check).
 *
 * Interrupts stay off, so no AGC clock tick or interrupt lands in a case
 * (the interpreter's yield on a tick, platform/c64/interp_module.c, is
 * covered by make vice-interp-core).
 *
 * Cases arrive as a stream in the REU at IF_STREAM: per case a count, the
 * superbank and the entry's index, then count (address, value) pairs that
 * turn the previous case's image (kept at IF_REF, zero at first) into this
 * one's, or a count of $FFFE and the whole image (2048 words: the flowed
 * states of tools/interp_flight_flowed.c share few words); a count of
 * $FFFF ends it. Results (12 bytes each, IF_MAX at most)
 * go to $c010; mailbox (tools/vice_run.py): $c001-2 failures, $c003-4
 * checks (cases run), $c005 the first failing step, $a5 at $c000, then
 * at $c00f (done; written last). */
#include <stdint.h>
#include "resident_flight.h"
#include IF_DATA
#define BOOT __attribute__((noinline,section(".boot.text.if_check")))
#define IMAGE ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)
typedef struct { uint32_t digest; uint16_t z, fb, steps; uint8_t status, flags; } if_result;
_Static_assert(sizeof(if_result)==12u,"results are 12 bytes (tools/interp_flight_capture.c)");
#define RESULTS ((if_result *)0xc010)
_Static_assert(0xc010ul+IF_MAX*sizeof(if_result)<=0xcc00ul,"results would reach the stack canary");
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;
static uint16_t failures,cases;
static uint8_t first_failure;
static const uint16_t points[]=IF_POINTS;
extern volatile uint16_t ov2_irq_count;

/* The digest tools/interp_flight_capture.c takes. */
static uint32_t digest(void){uint16_t a=0,b=0,i;
 for(i=0;i<2048u;++i){if(i>=024u&&i<=031u)continue;a+=IMAGE->words[i];b+=a;}
 a+=rk_device.superbank;b+=a;
 return (uint32_t)b<<16|a;}
static uint8_t at_point(void){uint8_t i;uint16_t z=IMAGE->words[5]&07777u;
 for(i=0;i<sizeof points/sizeof points[0];++i)if(z==points[i])return 1;
 return 0;}

static uint32_t at;
static uint16_t pairs[64];
/* The next case into the image (0 at the end of the stream). */
static uint8_t next_case(void){uint16_t head[3],n,k,i;
 apollo_reu_transfer(APOLLO_REU_FETCH,head,at,sizeof head);at+=sizeof head;
 if(head[0]==0xffffu)return 0;
 if (head[0] == 0xfffeu) {
     apollo_reu_transfer(APOLLO_REU_FETCH, IMAGE, at, sizeof *IMAGE);
     at += sizeof *IMAGE;
 } else
     apollo_reu_transfer(APOLLO_REU_FETCH, IMAGE, IF_REF, sizeof *IMAGE);
 if (head[0] != 0xfffeu)
     for (n = head[0]; n; n -= k) {
         k = n < 32u ? n : 32u;
         apollo_reu_transfer(APOLLO_REU_FETCH, pairs, at, (uint16_t)(k * 4u));
         at += k * 4u;
         for (i = 0; i < k; ++i) IMAGE->words[pairs[2 * i]] = pairs[2 * i + 1];
     }
 apollo_reu_transfer(APOLLO_REU_STASH,IMAGE,IF_REF,sizeof *IMAGE);
 rk_device.superbank=head[1];
 return 1;}

static void BOOT boot_more(void){uint16_t i;
 /* The mailbox and the results reuse the pinned module's RAM. */
 rk_pinned=0;
 for(i=0;i<sizeof *IMAGE;++i)((uint8_t *)IMAGE)[i]=0;
 apollo_reu_transfer(APOLLO_REU_STASH,IMAGE,IF_REF,sizeof *IMAGE);
 for(i=1;i<0xc00u;++i)if(i!=0xfu)mail[i]=0;}

int main(void){
 if(rk_boot()){first_failure=1;++failures;}
 else{
  __asm__ volatile("sei");
  boot_more();
  at=IF_STREAM;
  while(cases<IF_MAX&&next_case()){if_result *r=&RESULTS[cases];enum exec_status s;uint16_t n=0;
   rk_pending=0;rk_hardware.in_interrupt=0;rk_ticks=ov2_irq_count;
   do{
#ifdef RK_WORK_CLOCK
    rk_work=0;   /* the work clock held too: no tick from the case's own work */
#endif
    s=rk_resident_step(AGC_TRANSFER_NONE);++n;}while(s==EXEC_OK&&at_point()&&n<IF_STEPS);
   r->digest=digest();r->z=IMAGE->words[5];r->fb=IMAGE->words[4];r->steps=n;r->status=(uint8_t)s;
   r->flags=(uint8_t)(at_point()?1u:0u);
   if(s!=EXEC_OK||r->flags){++failures;if(!first_failure)first_failure=2;}
   ++cases;}
  __asm__ volatile("cli");}
 mail[5]=first_failure;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);
 mail[3]=(uint8_t)cases;mail[4]=(uint8_t)(cases>>8);
 mail[0]=0xa5;
 mail[0xf]=0xa5;   /* the monitor's watch: done */
 for(;;){}
}
