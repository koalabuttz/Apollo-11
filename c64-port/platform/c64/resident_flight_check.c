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
 * with RK_PART 1 (services), 2 (jobs) or 3 (idle), one program each.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000;
 * $c005 the first failing step, $c006-7 interrupts seen. */
#include <stdint.h>
#include "resident_flight.h"
#include "resident_flight_check.h"
#include "resident_kernel_cases.h"
#if RK_LM
#define JOB_PARTS 3u
#else
#define JOB_PARTS 1u
#endif
/* RK_PART 1, 2 or 3 checks only that module's rows (each program's test
 * code must fit beside the resident and in the task slot). */
#ifdef RK_PART
#define PART(p) (RK_PART==(p))
#else
#define PART(p) 1
#endif
extern volatile uint16_t ov2_irq_count;
static volatile uint8_t *const mail=(volatile uint8_t *)0xc000;
static uint16_t failures,checks;
static rk_case c;
static const rk_check_data none={0,0,0,0};
static const rk_check_data *rows;
static void expect(uint8_t ok,uint8_t step){++checks;if(!ok){++failures;if(!mail[5])mail[5]=step;}}
/* Case c.n from module id: 1 written, 0 not in that module, 2 runtime failure. */
static uint16_t call(uint8_t context,uint16_t module,uint16_t value){apollo_ov2_cont at,resume;uint8_t action;
 at.module=module;at.selector=0;
 if(apollo_ov2_run(&rk_runtime,context,at,&value,&action,&resume)!=APOLLO_OV2_OK||action!=APOLLO_OV2_DONE)return 2u;
 return value;}
static uint8_t load_case(uint16_t module){return (uint8_t)call(0,module,(uint16_t)(uintptr_t)&c);}
int main(void){apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
 uint16_t i,n,irq;uint8_t got;uint32_t d;
#if PART(2)
 uint8_t part=1;
#endif
 for(i=1;i<8u;++i)mail[i]=0;   /* $c000 only at the end: vice_run watches it */
 expect(rk_boot()==0u,1);
 /* The rows and (LM) the services programs' RESTARTS table, from the
  * task slot. */
 rows=(const rk_check_data *)(uintptr_t)call(APOLLO_OV2_TASK_CONTEXT,RK_MODULE_DATA,0);
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
 /* The dispatcher: no kernel routine at the probe point, so the step
  * resolves it through the store and runs the probe. */
 image->words[4]=RK_PROBE_FB;image->words[5]=RK_PROBE_LOC;rk_device.superbank=0;rk_hardware.in_interrupt=0;
 expect(rk_resident_step(AGC_TRANSFER_NONE)==EXEC_OK&&image->words[5]==RK_PROBE_Z,6);
 /* SPSIN through the resident's table: sin 0 is 0, and Z returns to Q. */
 image->words[0]=0;image->words[1]=0;image->words[2]=RK_SP_RETURN;image->words[4]=RK_PROBE_FB;image->words[5]=RK_SPSIN;image->words[7]=0;
 expect(rk_resident_step(agc_fixed_physical(RK_SP_RETURN,RK_PROBE_FB,0))==EXEC_OK&&image->words[5]==RK_SP_RETURN&&image->words[0]==0u&&image->words[2]==3u,9);
 /* Interrupts kept arriving through the resident vector. */
 expect(ov2_irq_count!=irq&&*(volatile uint8_t *)1==0x35,7);
 irq=ov2_irq_count;mail[6]=(uint8_t)irq;mail[7]=(uint8_t)(irq>>8);
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
