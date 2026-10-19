/* SPDX-License-Identifier: MIT */
/* resident-flight: the thin kernel in the linked ABI 2 flight resident on
 * the C64 (platform/c64/resident_flight.ld). The program boots like the
 * flight resident (KERNAL out, resident interrupt vectors live, the REU
 * store opened and verified), then for each C64 row of
 * tests/<m>_kernel_c64_rows.h (from the check's data module in the task
 * slot) loads the row's case module from the store
 * into the job slot, which writes the case into the image at $7000, and runs
 * the kernel's half in the resident against the frozen yaAGC digest (LM
 * services cases chain into RESTARTS' routines, from the data module). Last,
 * one resident step (rk_resident_step) from a point with no kernel routine
 * must resolve through the store to the services module's probe, and one
 * from SPSIN must return to Q through the resident's SPSIN/SPCOS table. Build
 * with RK_PART 1 (services), 2 (jobs), 3 (idle) or 4 (the AGC clock and
 * T3RUPT through resident steps, tests/resident_rupt_c64_rows.h), one
 * program each.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000;
 * $c005 the first failing step, $c006-7 interrupts seen. */
#include <stdint.h>
#include "resident_flight.h"
#include "resident_flight_check.h"
#include "resident_kernel_cases.h"
#include "resident_rupt.h"
#include "resident_rupt_c64_rows.h"
#include "resident_vector_run.h"
#if RK_LM
#define JOB_PARTS 3u
#define RUPT_ROWS rr_lm_c64_rows
#define CASE_IO lm
#else
#define JOB_PARTS 1u
#define RUPT_ROWS rr_cm_c64_rows
#define CASE_IO cm
#endif
/* RK_PART 1, 2 or 3 checks only that module's rows (each program's test
 * code must fit beside the resident and in the task slot); 4 checks the
 * AGC clock and T3RUPT through resident steps. */
#ifdef RK_PART
#define PART(p) (RK_PART==(p))
#else
#define PART(p) 1
#endif
extern volatile uint16_t ov2_irq_count;
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;
static uint16_t failures,checks;
static rk_case c __attribute__((unused));   /* unused in part 4 */
static const rk_check_data none={0,0,0,0};
static const rk_check_data *rows;
static void expect(uint8_t ok,uint8_t step){++checks;if(!ok){++failures;if(!mail[5])mail[5]=step;}}
/* Case c.n from module id (selector): 1 written, 0 not in that module, 2 runtime failure. */
static uint16_t call(uint8_t context,uint16_t module,uint8_t selector,uint16_t value){apollo_ov2_cont at,resume;uint8_t action;
 at.module=module;at.selector=selector;
 if(apollo_ov2_run(&rk_runtime,context,at,&value,&action,&resume)!=APOLLO_OV2_OK||action!=APOLLO_OV2_DONE)return 2u;
 return value;}
#if PART(1)||PART(2)||PART(3)
static uint8_t load_case(uint16_t module){return (uint8_t)call(0,module,0,(uint16_t)(uintptr_t)&c);}
#endif
#if PART(4)
/* T3RUPT rows (tests/resident_rupt_c64_rows.h): with the CIA interrupt
 * held off (and the work clock held), so no clock tick lands, the T3RUPT case module
 * (resident_flight_rupt_cases.c) writes each row's state, and with T3RUPT
 * pending it
 * runs through resident steps (the entry, the kernel's T3RUPT, its tasks
 * and TASKOVER) until RESUME brings Z back where it was; the image,
 * channel 7, the interrupt enable and the interrupt flag must give yaAGC's
 * digest. */
static void rupt_rows(apollo_overlay_state *image){static rk_rupt_row row;uint16_t r,i,stop;uint8_t guard;uint32_t h;
 __asm__ volatile("sei");
 rk_ticks=ov2_irq_count;            /* drop the ticks since boot */
 for(r=0;r<RR_C64_ROWS;++r){
  row.n=(uint16_t)(r*RR_C64_STRIDE);
  if(call(0,RK_MODULE_RUPT,0,(uint16_t)(uintptr_t)&row)!=1u){expect(0,10);continue;}
  rk_device.superbank=row.c.superbank;rk_device.interrupt_enabled=row.c.interrupt_enabled;rk_hardware.in_interrupt=0;
  rk_pending=RK_RUPT_BIT(RK_RUPT_T3);
  stop=agc_transfer_point(&rk_program,image,rk_device.superbank);
  for(guard=0;guard<64u;++guard){
#ifdef RK_WORK_CLOCK
   rk_work=0;   /* the work clock held too: no tick from the rows' own work */
#endif
   if(rk_resident_step(stop)!=EXEC_OK)break;
   if(!rk_hardware.in_interrupt&&agc_transfer_point(&rk_program,image,rk_device.superbank)==stop)break;}
  h=rr_mix(RR_BASIS,(uint16_t)(guard<64u&&!rk_hardware.in_interrupt));
  for(i=0;i<2048u;++i)h=rr_mix(h,image->words[i]);
  h=rr_mix(h,rk_device.superbank);h=rr_mix(h,rk_device.interrupt_enabled);h=rr_mix(h,rk_hardware.in_interrupt);
  expect(h==RUPT_ROWS[r],10);}
 __asm__ volatile("cli");
}
/* The AGC clock: after five or more CIA interrupts the step takes
 * RK_TICK_LAG of them (TIME1) and drops the rest into rk_dilation, so the
 * two together count exactly the interrupts consumed; CIA1 timer A must be
 * counting from the latch. */
#ifdef RK_WORK_CLOCK
/* The work clock: two whole periods of work (854 and 853 MCT, from phase 0)
 * and five MCT more become two periods' six steps (TIME1 twice) at the
 * next step, with interrupts inhibited, the five carried into the third
 * period; a step that runs nothing adds no work. CIA1 timer A still counts
 * from the latch. */
static void clock_check(apollo_overlay_state *image){
 image->words[025]=0;rk_device.interrupt_enabled=0;
 rk_resident_step(agc_transfer_point(&rk_program,image,rk_device.superbank));
 image->words[025]=0;rk_tick_phase=0;rk_work=854u+853u+5u;
 rk_resident_step(agc_transfer_point(&rk_program,image,rk_device.superbank));
 expect(image->words[025] == 2u && rk_work == 5u && rk_tick_phase == 6u, 11);
 expect(*(volatile uint8_t *)0xdc05<=(uint8_t)(RK_CIA_LATCH>>8),12);
 /* The real-time cap: with no lag owed, three periods of work take three
  * ticks only as CIA1 counts three more periods. */
 {
     uint8_t c = *(volatile uint8_t *)&ov2_irq_count;
     image->words[025] = 0;
     rk_tick_phase = 0;
     rk_work = 2560u;
     rk_cap = c;
     rk_resident_step(agc_transfer_point(&rk_program, image, rk_device.superbank));
     expect(image->words[025] == 3u && (uint8_t)(*(volatile uint8_t *)&ov2_irq_count - c) >= 3u,
            19);
 }
}
/* Nine periods of work (7,680 MCT) with T6, T5, T4 and T3 all coming due
 * in the first: the step stops at the first part of the period (2.5 ms,
 * 214 MCT), where T6 comes due, and enters T6RUPT; T4 is left pending, and
 * TIME1, TIME3 and TIME5 wait for the rest of the work, none dropped. The
 * scalers count the part, SCALER2 on SCALER1's overflow. With T6 served,
 * the next step enters T4RUPT before any more time passes, so T3, which
 * outranks it, is not yet requested (the AGC's order); the step after
 * that takes the 5 ms part and enters T3RUPT. */
static void lag_check(apollo_overlay_state *image){uint32_t d;uint16_t t6=agc_fixed_physical(04004u,image->words[4],0);
 __asm__ volatile("sei");
 d=rk_dilation;rk_tick_phase=0;rk_work=3u*2560u;
 rk_cap =
     (uint8_t)(ov2_irq_count - 9u); /* nine periods of lag owed: the cap lets all nine through */
 rk_pending=0;rk_hardware.in_interrupt=0;rk_device.interrupt_enabled=1;rk_device.superbank=0;
 image->words[0]=0;image->words[5]=0100u;image->words[0100]=030000u;   /* Z erasable, CA 0: not RELINT/INHINT/EXTEND */
 image->words[030] = 037777u;
 image->words[031] = 1u;
 RK_TIMER_CH13 = 040000u;
 image->words[025] = 0;
 image->words[026] = 037777u;
 image->words[027] = 037777u;
 rk_hardware.channel4 = 037770u;
 rk_hardware.channel3 = 037777u; /* SCALER1 overflows in the first part, and SCALER2 with it */
 expect(rk_resident_step(t6) == EXEC_OK && image->words[5] == 04004u && rk_hardware.in_interrupt &&
            rk_pending == RK_RUPT_BIT(RK_RUPT_T4) && image->words[015] == 0101u &&
            image->words[025] == 0u && image->words[026] == 037777u &&
            image->words[030] == 037777u && rk_work == 3u * 2560u - 214u && rk_tick_phase == 1u &&
            rk_dilation == d && !(RK_TIMER_CH13 & 040000u) && rk_hardware.channel4 == 0u &&
            rk_hardware.channel3 == 0u,
        18);
 rk_hardware.in_interrupt = 0;
 image->words[5] = 0100u;
 expect(rk_resident_step(agc_fixed_physical(04020u, image->words[4], 0)) == EXEC_OK &&
            image->words[5] == 04020u && rk_hardware.in_interrupt && rk_pending == 0u &&
            image->words[025] == 0u && image->words[026] == 037777u &&
            rk_work == 3u * 2560u - 214u && rk_tick_phase == 1u,
        20);
 rk_hardware.in_interrupt = 0;
 image->words[5] = 0100u;
 expect(rk_resident_step(agc_fixed_physical(04014u, image->words[4], 0)) == EXEC_OK &&
            image->words[5] == 04014u && rk_hardware.in_interrupt && rk_pending == 0u &&
            image->words[025] == 1u && image->words[026] == 0u &&
            rk_work == 3u * 2560u - 214u - 213u && rk_tick_phase == 2u &&
            rk_hardware.channel4 == 8u,
        21);
 rk_pending = 0;
 rk_hardware.in_interrupt = 0;
 RK_TIMER_CH13 = 0;
 __asm__ volatile("cli");
}
#else
static void clock_check(apollo_overlay_state *image){uint16_t t,irq;uint32_t d;
 image->words[025]=0;rk_device.interrupt_enabled=0;
 rk_resident_step(agc_transfer_point(&rk_program,image,rk_device.superbank));
 image->words[025]=0;t=rk_ticks;d=rk_dilation;
 /* The count is read twice each time: one read between its two bytes'
  * carry can tear (it ended the wait early in the x0.5 build). */
 do irq=ov2_irq_count; while(irq!=ov2_irq_count);
 for(;;){uint16_t a=ov2_irq_count;if(a==ov2_irq_count&&(uint16_t)(a-irq)>=5u)break;}
 rk_resident_step(agc_transfer_point(&rk_program,image,rk_device.superbank));
 expect((uint16_t)(rk_ticks-t)>=5u&&image->words[025]==RK_TICK_LAG&&
        rk_dilation-d==(uint16_t)(rk_ticks-t-RK_TICK_LAG),11);
 expect(*(volatile uint8_t *)0xdc05<=(uint8_t)(RK_CIA_LATCH>>8),12);
}
/* A backlog past the cap with T5 and T6 both coming due in the first tick:
 * one step takes RK_TICK_LAG ticks, drops the rest, and enters T6RUPT
 * alone (AGC priority), with T5RUPT left pending for after its RESUME. The
 * scalers count the periods taken, SCALER2 on SCALER1's overflow. */
static void lag_check(apollo_overlay_state *image){uint32_t d;uint16_t t6=agc_fixed_physical(04004u,image->words[4],0);
 __asm__ volatile("sei");
 rk_ticks=(uint16_t)(ov2_irq_count-RK_TICK_LAG-7u);d=rk_dilation;
 rk_pending=0;rk_hardware.in_interrupt=0;rk_device.interrupt_enabled=1;rk_device.superbank=0;
 image->words[0]=0;image->words[5]=0100u;image->words[0100]=030000u;   /* Z erasable, CA 0: not RELINT/INHINT/EXTEND */
 image->words[030] = 037777u;
 image->words[031] = 1u;
 RK_TIMER_CH13 = 040000u;
 image->words[025]=0;image->words[026]=0;image->words[027]=0;
 rk_hardware.channel4=037740u;rk_hardware.channel3=037777u;   /* SCALER1 overflows in the first period, and SCALER2 with it */
 expect(rk_resident_step(t6) == EXEC_OK && image->words[5] == 04004u && rk_hardware.in_interrupt &&
            rk_pending == RK_RUPT_BIT(RK_RUPT_T5) && image->words[015] == 0101u &&
            image->words[025] == RK_TICK_LAG && image->words[026] == RK_TICK_LAG &&
            rk_dilation - d == 7u && rk_ticks == ov2_irq_count && !(RK_TIMER_CH13 & 040000u) &&
            rk_hardware.channel4 == (uint16_t)(32u * RK_TICK_LAG - 32u) &&
            rk_hardware.channel3 == 0u,
        18);
 rk_pending = 0;
 rk_hardware.in_interrupt = 0;
 RK_TIMER_CH13 = 0;
 __asm__ volatile("cli");
}
#endif
#endif
#if PART(5)
/* Other-vector rows (tests/resident_rupt_c64_rows.h): with the CIA
 * interrupt held off (and the work clock held), the other-vector case module
 * (resident_flight_vector_cases.c) writes each row's state, and with that
 * vector pending it runs through resident steps (the entry, the lead-in and
 * the interrupt body from their task-slot wrappers, the kernel where a body
 * calls it) until RESUME brings Z back where it was or a step finds no
 * routine and no module (yaAGC ran the original to the same point); then
 * the module checks the run's digest against yaAGC's. The row lives in
 * free RAM above the mailbox and the boot scratch: the check has no room
 * for it beside the resident. */
#define VECTOR_ROW ((rk_vector_row *)0xc200)
typedef char vector_row_fits[sizeof(rk_vector_row)<=RK_STACK_CANARY-0xc200u?1:-1];
static void vector_rows(apollo_overlay_state *image){rk_vector_row *const row=VECTOR_ROW;uint16_t r,stop,n;
 enum exec_status st;uint8_t kind;
 __asm__ volatile("sei");
 rk_ticks=ov2_irq_count;
 for(r=0;r<RR_C64_VROWS;++r){
  row->n=(uint16_t)(r*RR_C64_VSTRIDE);
  if(call(0,RK_MODULE_VECTOR,0,(uint16_t)(uintptr_t)row)!=1u){expect(0,16);continue;}
  rk_device=row->c.CASE_IO;rk_hardware=row->c.hw;rk_pending=row->c.pending;
#if RK_CM
  rk_channels_seed(); /* the row's channels are rk_hardware's */
#endif
  stop=agc_transfer_point(&rk_program,image,rk_device.superbank);
  kind=RR_V_RUNAWAY;
  for(n=0;n<RR_V_MAX;++n){
#ifdef RK_WORK_CLOCK
   rk_work=0;   /* the work clock held too */
#endif
   st=rk_resident_step(stop);
   if(st==EXEC_UNMAPPED){kind=RR_V_STOPPED;break;}
   if(st!=EXEC_OK){kind=RR_V_REFUSED;break;}
   if(!rk_hardware.in_interrupt&&agc_transfer_point(&rk_program,image,rk_device.superbank)==stop){kind=RR_V_RESUMED;break;}}
  if(!n&&rk_pending)kind=RR_V_NOT_TAKEN;
  row->c.CASE_IO=rk_device;row->c.hw=rk_hardware;row->kind=kind;
  expect(call(0,RK_MODULE_VECTOR,1,(uint16_t)(uintptr_t)row)==1u,16);}
 rk_pending=0;
 __asm__ volatile("cli");
}
/* The boundary-word table at the REU store's tail (rk_boundary_entry):
 * every key gives its word through rk_fixed_word and the next key that is
 * not in the table gives none; at a fixed-fixed boundary the entry stores
 * that word in BRUPT, and at a fixed point missing from the table it takes
 * nothing and the request stays pending. */
static void boundary_check(apollo_overlay_state *image){uint16_t i,k;agc_word w;uint8_t ok=1,taken=0;
 agc_site_operand e;const agc_site_operand *at;
 for(i=0;i<rk_boundary_count;++i){
  if(!(at=rk_boundary_entry(i))){ok=0;break;}
  e=*at;k=e.physical;
  if(!rk_fixed_word(&rk_program,k,&w)||w!=e.word)ok=0;
  if((!(at=rk_boundary_entry((uint16_t)(i+1u)))||at->physical!=k+1u)&&rk_fixed_word(&rk_program,(uint16_t)(k+1u),&w))ok=0;}
 expect(ok&&rk_boundary_count>0u,13);
 image->words[0]=0;image->words[4]=RK_PROBE_FB;rk_device.superbank=0;rk_hardware.in_interrupt=0;
 for(i=0;(at=rk_boundary_entry(i));++i){k=at->physical;w=at->word;
  if(k<04000u||k>=010000u||w==3u||w==4u||w==6u)continue;   /* fixed-fixed: Z is the key */
  image->words[5]=(agc_word)k;rk_pending=RK_RUPT_BIT(RK_RUPT_T3);rk_hardware.in_interrupt=0;
  taken=(uint8_t)(rk_rupt_take(&rk_program,image,0,1,&rk_pending)==RK_RUPT_T3&&image->words[017]==w&&
   image->words[015]==(agc_word)(k+1u)&&image->words[5]==04014u&&!rk_pending);
  break;}
 expect(taken,14);
 image->words[5]=RK_PROBE_FF_LOC+1u;rk_pending=RK_RUPT_BIT(RK_RUPT_T3);rk_hardware.in_interrupt=0;
 expect(!rk_rupt_take(&rk_program,image,0,1,&rk_pending)&&rk_pending==RK_RUPT_BIT(RK_RUPT_T3)&&!rk_hardware.in_interrupt,14);
 rk_pending=0;
}
#endif
int main(void){apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
 uint16_t i,n,irq;uint32_t d;
#if PART(1)||PART(2)||PART(3)
 uint8_t got;
#endif
#if PART(2)
 uint8_t part=1;
#endif
 for(i=1;i<8u;++i)mail[i]=0;   /* $c000 only at the end: vice_run watches it */
 expect(rk_boot()==0u,1);
 /* The rows and (LM) the services programs' RESTARTS table, from the
  * task slot. */
 rows=(const rk_check_data *)(uintptr_t)call(APOLLO_OV2_TASK_CONTEXT,RK_MODULE_DATA,0,0);
 expect((uintptr_t)rows>=APOLLO_OV2_TASK_ADDRESS,8);
 if((uintptr_t)rows<APOLLO_OV2_TASK_ADDRESS)rows=&none;
#if RK_LM
 c.restart=rows->restart;
#endif
 irq=ov2_irq_count;
 for(i=0;i<rows->rows;++i){n=rows->cases[i];
  if(n<RK_SERVICES_CASES){
#if PART(1)
   c.n=n;got=load_case(RK_MODULE_SERVICES);
   expect(got==1u,2);d=got==1u?rk_services_run(image,&c):0;
#else
   continue;
#endif
  }else if((n=(uint16_t)(n-RK_SERVICES_CASES))<RK_JOBS_CASES){
#if PART(2)
   for(;;){c.n=n;got=load_case(RK_MODULE_JOBS(part));if(got||part>=JOB_PARTS)break;++part;}
   expect(got==1u,3);d=got==1u?rk_jobs_run(image,&c):0;
#else
   continue;
#endif
  }else{
#if PART(3)
   c.n=(uint16_t)(n-RK_JOBS_CASES);got=load_case(RK_MODULE_IDLE);
   expect(got==1u,4);d=got==1u?rk_idle_run(image,&c):0;
#else
   continue;
#endif
  }
  expect(d==rows->digests[i],5);}
#if PART(4)
 rupt_rows(image);
 clock_check(image);
 lag_check(image);
#endif
#if PART(5)
 vector_rows(image);
 boundary_check(image);
#endif
 /* The dispatcher: no kernel routine at the probe point, so the step
  * resolves it through the store and runs the probe (no interrupt is
  * taken: the probe is a boundary in the table). */
 rk_device.interrupt_enabled=0;rk_pending=0;
 image->words[4]=RK_PROBE_FB;image->words[5]=RK_PROBE_LOC;rk_device.superbank=0;rk_hardware.in_interrupt=0;
 /* The stack canary: a step refuses to run while it is changed. The case
  * modules' input generators run below it (the CM job cases reach $C800,
  * a test-only depth; this store has no pinned module), so the check sets
  * it again before the steps. */
 *(volatile uint8_t *)RK_STACK_CANARY=(uint8_t)~RK_STACK_CANARY_VALUE;
 expect(rk_resident_step(AGC_TRANSFER_NONE) == EXEC_OUT_OF_SCOPE && image->words[5] == RK_PROBE_LOC,
        17);
 *(volatile uint8_t *)RK_STACK_CANARY=RK_STACK_CANARY_VALUE;
 expect(rk_resident_step(AGC_TRANSFER_NONE)==EXEC_OK&&image->words[5]==RK_PROBE_Z,6);
 /* At fixed-fixed the resolver key is the canonical bank, whatever FB and
  * the superbank hold. */
 image->words[4]=036000u;image->words[5]=RK_PROBE_FF_LOC;rk_device.superbank=0160u;
 expect(rk_resident_step(AGC_TRANSFER_NONE)==EXEC_OK&&image->words[5]==RK_PROBE_Z,15);
 rk_device.superbank=0;
 /* SPSIN through the resident's table: sin 0 is 0, and Z returns to Q. */
 image->words[0]=0;image->words[1]=0;image->words[2]=RK_SP_RETURN;image->words[4]=RK_PROBE_FB;image->words[5]=RK_SPSIN;image->words[7]=0;
 expect(rk_resident_step(agc_fixed_physical(RK_SP_RETURN,RK_PROBE_FB,0))==EXEC_OK&&image->words[5]==RK_SP_RETURN&&image->words[0]==0u&&image->words[2]==3u,9);
 /* The TC trap's restart (rk_gojam, resident_flight_main.c): Q takes Z,
  * Z goes to 04000, every interrupt request is dropped and the RESTART
  * lamp lights. */
 image->words[4] = RK_PROBE_FB;
 image->words[5] = RK_PROBE_LOC;
 rk_pending = RK_RUPT_BIT(RK_RUPT_T3);
 expect(rk_gojam() == 0u && image->words[5] == 04000u && image->words[2] == RK_PROBE_LOC &&
            rk_pending == 0u,
        18);
 /* Interrupts kept arriving through the resident vector. */
 expect(ov2_irq_count!=irq&&*(volatile uint8_t *)1==0x35,7);
 irq=ov2_irq_count;mail[6]=(uint8_t)irq;mail[7]=(uint8_t)(irq>>8);
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
