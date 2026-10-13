/* SPDX-License-Identifier: MIT */
/* Prints the physical key of every fixed routine in the flight resident's
 * routine tables and in the interrupt bodies its task-slot wrappers run
 * (tools/resident_boundary_words.py reads it): "<table> <key>" per line,
 * keys in decimal. Build with -DRK_LM=1 or -DRK_CM=1. */
#include <stdio.h>
#include "resident_kernel.h"
#include "sp_routines.h"
#include "rupt_lead_ins.h"
#include "t4rupt_core.h"
#include "keyrupt_uprupt.h"
#if RK_LM
#include "lm_t6rupt_routines.h"
#endif
static void show(const char *name,const agc_routine_table *t){uint16_t i;
 for(i=0;i<t->count;++i)if(t->entries[i].physical<AGC_TRANSFER_ERASABLE_BASE)printf("%s %u\n",name,t->entries[i].physical);}
int main(void){
#if RK_LM
 show("lm_kernel_routines",&lm_kernel_routines);show("lm_kernel_job_routines",&lm_kernel_job_routines);
 show("lm_kernel_task_routines",&lm_kernel_task_routines);show("lm_kernel_idle_routines",&lm_kernel_idle_routines);
 show("lm_sp_routines",&lm_sp_routines);show("lm_lead_in_routines",&lm_lead_in_routines);
 show("lm_t6rupt_routines",&lm_t6rupt_routines);show("lm_t4rupt_core_routines",&lm_t4rupt_core_routines);
 show("lm_keyrupt_routines",&lm_keyrupt_routines);
#else
 show("cm_kernel_routines",&cm_kernel_routines);show("cm_kernel_job_routines",&cm_kernel_job_routines);
 show("cm_kernel_idle_routines",&cm_kernel_idle_routines);show("cm_sp_routines",&cm_sp_routines);
 show("cm_lead_in_routines",&cm_lead_in_routines);show("cm_t4rupt_core_routines",&cm_t4rupt_core_routines);
 show("cm_keyrupt_routines",&cm_keyrupt_routines);
#endif
 return 0;}
