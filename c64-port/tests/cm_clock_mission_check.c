/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission_cases.h"
#include "cm_clock_mission.h"
#include "cm_clock_mission_check.h"
#include "cm_clock_mission_vectors.h"
static apollo_overlay_state s;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_clock_mission_check(void){unsigned run,n,step,i,fail=0;
 for(run=0;run<
#ifdef __mos__
 64u
#else
 448u
#endif
 ;++run){cm_mission_loop r={&s,&io,cm_clock_mission_builtin_invoke,0,{0,0},{{0,0},0},0,1,0};
#ifdef __mos__
 n=cm_clock_mission_indices[run];
#else
 n=run;
#endif
 cm_clock_mission_input(&s,&io,(uint16_t)n);r.pending_t3=(uint8_t)(n<384u);
 for(step=0;step<cm_clock_mission_count[n];++step){cm_mission_call call;enum cm_service_status status;uint32_t h;
 status=cm_clock_mission_identify(&r,&call);if(status!=CM_SERVICE_ADVANCED){++fail;break;}status=cm_clock_mission_step(&r);
 h=fold(2166136261u,(uint16_t)call.group);h=fold(h,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,s.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);h=fold(h,r.irq.in_isr);h=fold(h,r.irq.pending_danzig);h=fold(h,r.transfer.interpretive);h=fold(h,r.transfer.address.location);h=fold(h,r.transfer.address.bank);
 if(h!=cm_clock_mission_hash[run][step]||status!=CM_SERVICE_ADVANCED)++fail;
 if(n==0&&step+1u==cm_clock_mission_count[0]){if(cm_clock_mission_sample[0]!=call.group||cm_clock_mission_sample[1]!=(unsigned)status)++fail;for(i=0;i<2048u;++i)if(cm_clock_mission_sample[i+2u]!=s.words[i])++fail;if(cm_clock_mission_sample[2050]!=io.superbank||cm_clock_mission_sample[2051]!=io.interrupt_enabled||cm_clock_mission_sample[2052]!=r.irq.in_isr||cm_clock_mission_sample[2053]!=r.irq.pending_danzig||cm_clock_mission_sample[2054]!=r.transfer.interpretive||cm_clock_mission_sample[2055]!=r.transfer.address.location||cm_clock_mission_sample[2056]!=r.transfer.address.bank)++fail;}
 }if(r.in_callback||r.completed_groups!=cm_clock_mission_count[n]||r.irq.in_isr||r.irq.pending_danzig||r.pending_t3)++fail;
 }return fail;}
