/* SPDX-License-Identifier: MIT */
/* An interrupt-body wrapper (ABI 2 task slot): one of the Kernel lane's
 * routine tables, unchanged, run on the transfer ABI from the point the
 * step resolved to this module until a point the tables do not hold
 * (docs/RESIDENT-FLIGHT.md). Build with RK_WRAP:
 *   1  the lead-ins (rupt_lead_ins.h; CM with the RESUME family)
 *   2  the T4RUPT core (t4rupt_core.h)
 *   3  KEYRUPT and UPRUPT (keyrupt_uprupt.h)
 *   4  LM T6RUPT and the LM RESUME family (lm_t6rupt_routines.h)
 * Value: the step's rk_module_env; the routines get the resident's program
 * (hardware, fixed_data, site operands) and io. */
#include "resident_flight.h"
#if RK_WRAP==1
#include "rupt_lead_ins.h"
#elif RK_WRAP==4 && RK_LM
#include "lm_t6rupt_routines.h"
#elif RK_WRAP==2
#include "t4rupt_core.h"
#elif RK_WRAP==3
#include "keyrupt_uprupt.h"
#else
#error RK_WRAP must be 1, 2, 3 or (LM) 4
#endif
#define RK_WRAP_STEPS 256u
#if RK_LM
#define TABLE(name) lm_##name
#else
#define TABLE(name) cm_##name
#endif
/* The routine at a physical fixed address, by binary search of one table. */
static agc_routine find(const agc_routine_table *t,uint16_t physical){
 uint16_t low=0,high=t->count,middle;
 while(low<high){middle=(uint16_t)(low+((high-low)>>1));
  if(t->entries[middle].physical==physical)return t->entries[middle].call;
  if(t->entries[middle].physical<physical)low=(uint16_t)(middle+1u);else high=middle;}
 return 0;
}
/* The run: the tables' routine at Z, then at its continuation, and so on
 * (agc_transfer_run's loop, without its general resolver: every
 * interrupt body is in fixed memory). Handed back at a point no table
 * holds, or after RK_WRAP_STEPS routines at one they do, which the next
 * step resolves here again. A first point no table holds, or a routine's
 * failure, goes to env->status with Z left where it stopped. */
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 rk_io *io=(rk_io *)e->io;
 apollo_overlay_state *s=f->state;
 uint16_t n;
 (void)selector;
 for(n=0;n<RK_WRAP_STEPS;++n){
  agc_routine call;
  enum exec_status status;
  uint16_t physical=agc_fixed_physical(s->words[5],s->words[4],io->superbank);
#if RK_WRAP==1
  call=find(&TABLE(lead_in_routines),physical);
#elif RK_WRAP==4
  call=find(&lm_t6rupt_routines,physical);
#elif RK_WRAP==2
  call=find(&TABLE(t4rupt_core_routines),physical);
#else
  call=find(&TABLE(keyrupt_routines),physical);
#endif
  if(!call){if(!n)e->status=EXEC_UNMAPPED;break;}
  status=call(e->program,s,io);
  if(status!=EXEC_OK){e->status=status;break;}
 }
 return APOLLO_OV2_DONE;
}
