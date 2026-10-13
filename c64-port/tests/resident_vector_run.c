/* SPDX-License-Identifier: MIT */
/* Build with -DRK_LM=1 or -DRK_CM=1 (resident_vector_run.h). */
#include "resident_vector_run.h"
#include "resident_rupt.h"
#include "resident_kernel.h"
#include "sp_routines.h"
#include "rupt_lead_ins.h"
#include "t4rupt_core.h"
#include "keyrupt_uprupt.h"
#if RK_LM
#include "lm_exec_routines.h"
#include "lm_t6rupt_routines.h"
#define RESIDENT 5u
#define TABLES 9u
#else
#include "cm_exec_routines.h"
#define RESIDENT 4u
#define TABLES 7u
#endif
static agc_routine_table tables[TABLES];
const agc_routine_table *rr_vector_tables(unsigned mission,uint8_t *count,uint8_t *resident){(void)mission;
#if RK_LM
 tables[0]=lm_kernel_routines;tables[1]=lm_kernel_job_routines;tables[2]=lm_kernel_task_routines;
 tables[3]=lm_kernel_idle_routines;tables[4]=lm_sp_routines;
 tables[5]=lm_lead_in_routines;tables[6]=lm_t6rupt_routines;tables[7]=lm_t4rupt_core_routines;tables[8]=lm_keyrupt_routines;
#else
 tables[0]=cm_kernel_routines;tables[1]=cm_kernel_job_routines;tables[2]=cm_kernel_idle_routines;tables[3]=cm_sp_routines;
 tables[4]=cm_lead_in_routines;tables[5]=cm_t4rupt_core_routines;tables[6]=cm_keyrupt_routines;
#endif
 *count=TABLES;*resident=RESIDENT;return tables;}
uint8_t rr_vector_run(unsigned mission,rr_vcase *c,rr_fixed fixed,uint16_t *point,uint16_t *routines){
 agc_native_program p={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint8_t count,resident;uint16_t stop,n;
 void *io=mission?(void *)&c->cm:(void *)&c->lm;agc_word *sb=mission?&c->cm.superbank:&c->lm.superbank;
 agc_word ie=mission?c->cm.interrupt_enabled:c->lm.interrupt_enabled;
 p.basic=rr_vector_tables(mission,&count,&resident);p.basic_count=count;p.hardware=&c->hw;p.fixed_data=fixed;
#if RK_LM
 p.operands=lm_exec_site_table(&p.operand_count);
#else
 p.operands=cm_exec_site_table(&p.operand_count);
#endif
 *routines=0;
 stop=agc_transfer_point(&p,c->s,*sb);*point=stop;
 if(!rk_rupt_take(&p,c->s,*sb,ie,&c->pending))return RR_V_NOT_TAKEN;
 for(n=0;n<RR_V_MAX;++n){agc_routine call;enum exec_status status;
  *point=agc_transfer_point(&p,c->s,*sb);
  if(!c->hw.in_interrupt&&*point==stop)return RR_V_RESUMED;
  call=agc_transfer_resolve(&p,c->s,*sb);
  if(!call)return RR_V_STOPPED;
  status=call(&p,c->s,io);++*routines;
  if(status!=EXEC_OK)return RR_V_REFUSED;}
 return RR_V_RUNAWAY;}
