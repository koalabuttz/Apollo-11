/* SPDX-License-Identifier: MIT */
#include "resident_kernel_cases.h"
#if RK_LM
#include "lm_restart_routines.h"
#endif
agc_word rk_entry;
#ifdef RK_RUNS_ONLY
#define RK_INPUT 0
#else
#define RK_INPUT 1
#endif
#ifdef RK_INPUTS_ONLY
#define RK_RUN 0
#else
#define RK_RUN 1
#endif
#if RK_RUN
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
/* The milestones' digest: outcome kind, then (unless refused) the image
 * with A, L (and Q where wide16 says) in 16 bits, then the device words. */
static uint32_t digest(const apollo_overlay_state *s,unsigned kind,unsigned wide16,const uint16_t *dev,unsigned ndev){unsigned i;
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=2u){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<wide16?0177777u:077777u)));for(i=0;i<ndev;++i)mix(dev[i]);}
 return (uint32_t)sum_b<<16|sum_a;
}
static agc_native_program program(void){agc_native_program p={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};return p;}
#endif

/* RK_PART (C64 programs) builds one module: 1 services, 2 jobs, 3 idle.
 * RK_INPUTS_ONLY builds only the inputs (a C64 case module), RK_RUNS_ONLY
 * only the kernel halves (the C64 flight check). */
#ifndef RK_PART
#define RK_SERVICES 1
#define RK_JOBS 1
#define RK_IDLE 1
#else
#define RK_SERVICES (RK_PART==1)
#define RK_JOBS (RK_PART==2)
#define RK_IDLE (RK_PART==3)
#endif

#if RK_LM
#define ANY_FB 0177777u
#if RK_SERVICES&&RK_INPUT
void rk_services_input(apollo_overlay_state *s,rk_case *c){lm_exs_input(s,&c->io,&c->k.services,c->n);rk_entry=s->words[5];}
#endif
#if RK_SERVICES&&RK_RUN
uint32_t rk_services_run(apollo_overlay_state *s,rk_case *c){uint16_t steps;lm_exs_case *k=&c->k.services;agc_routine_table tables[2];agc_native_program p=program();enum exec_status st;unsigned kind;
 tables[0]=lm_kernel_routines;tables[1]=*c->restart;p.basic=tables;p.basic_count=2;p.operands=lm_exec_site_table(&p.operand_count);
 st=agc_transfer_run(&p,s,&c->io,&k->superbank,agc_fixed_physical(k->stop_z,k->stop_fb==ANY_FB?0u:k->stop_fb,k->superbank),48,&steps);
 kind=st==EXEC_OK?0u:st==EXEC_UNMAPPED&&(s->words[5]==LM_EXEC_BAILOUT1||s->words[5]==LM_EXEC_POODOO1)?1u:st==EXEC_OUT_OF_SCOPE?2u:3u;
 return digest(s,kind,2u,0,0);
}
#endif
#if RK_JOBS&&RK_INPUT
void rk_jobs_input(apollo_overlay_state *s,rk_case *c){lm_exj_input(s,&c->io,&c->k.jobs,c->n);rk_entry=s->words[5];}
#endif
#if RK_JOBS&&RK_RUN
uint32_t rk_jobs_run(apollo_overlay_state *s,rk_case *c){uint16_t steps,i;p66_bias_io *io=&c->io;lm_exj_case *k=&c->k.jobs;agc_routine_table tables[2];agc_native_program p=program();enum exec_status st;unsigned kind=3u;uint16_t dev[4];
 tables[0]=lm_kernel_job_routines;tables[1]=lm_kernel_task_routines;p.basic=tables;p.basic_count=2;p.operands=lm_exec_site_table(&p.operand_count);
 p.hardware=&k->hw;
 st=agc_transfer_run(&p,s,io,&io->superbank,k->until?agc_fixed_physical(k->until,0,0):AGC_TRANSFER_NONE,8,&steps);
 if(st==EXEC_OUT_OF_SCOPE)kind=2u;else if(st==EXEC_OK)kind=0u;
 else if(st==EXEC_UNMAPPED){agc_word z=s->words[5];if(z==LM_EXEC_BAILOUT1||z==LM_EXEC_POODOO1)kind=1u;else for(i=0;i<k->stops;++i)if(z==k->stop[i])kind=0u;}
 dev[0]=io->superbank;dev[1]=io->interrupt_enabled;dev[2]=k->hw.channel11;dev[3]=k->hw.in_interrupt;
 return digest(s,kind,3u,dev,4);
}
#endif
#if RK_IDLE&&RK_INPUT
void rk_idle_input(apollo_overlay_state *s,rk_case *c){lm_exi_input(s,&c->io,&c->k.idle,c->n);rk_entry=s->words[5];}
#endif
#if RK_IDLE&&RK_RUN
uint32_t rk_idle_run(apollo_overlay_state *s,rk_case *c){p66_bias_io *io=&c->io;agc_hardware *hw=&c->k.idle;agc_native_program p=program();agc_routine call;unsigned kind=3u;uint16_t dev[4];
 p.basic=&lm_kernel_idle_routines;p.basic_count=1;p.hardware=hw;
 call=agc_transfer_resolve(&p,s,io->superbank);
 if(call)kind=call(&p,s,io)!=EXEC_OK?2u:s->words[5]==LM_EXEC_BAILOUT1||s->words[5]==LM_EXEC_POODOO1?1u:0u;
 dev[0]=io->superbank;dev[1]=io->interrupt_enabled;dev[2]=hw->channel11;dev[3]=hw->in_interrupt;
 return digest(s,kind,3u,dev,4);
}
#endif
#else
#if RK_SERVICES&&RK_INPUT
void rk_services_input(apollo_overlay_state *s,rk_case *c){cm_exs_input(s,&c->io,&c->services,c->n);rk_entry=s->words[5];}
#endif
#if RK_SERVICES&&RK_RUN
uint32_t rk_services_run(apollo_overlay_state *s,rk_case *c){uint16_t steps,i;cm_native_io *io=&c->io;cm_exs_case *k=&c->services;agc_native_program p=program();enum exec_status st;unsigned kind=3u;uint16_t dev[3],stop=AGC_TRANSFER_NONE;
 p.basic=&cm_kernel_routines;p.basic_count=1;p.operands=cm_exec_site_table(&p.operand_count);
 p.hardware=&k->hw;
 if(k->resume||k->stops==1u)stop=agc_fixed_physical(k->stop_z[0],k->stop_fb[0]==CM_EXS_ANY_FB?0u:k->stop_fb[0],io->superbank);
 st=agc_transfer_run(&p,s,io,&io->superbank,stop,48,&steps);
 if(st==EXEC_OUT_OF_SCOPE)kind=2u;else if(st==EXEC_OK)kind=0u;
 else if(st==EXEC_UNMAPPED){agc_word z=s->words[5],fb=s->words[4];if(z==CM_EXEC_BAILOUT||z==CM_EXEC_POODOO)kind=1u;
  else for(i=0;i<k->stops;++i)if(z==k->stop_z[i]&&(k->stop_fb[i]==CM_EXS_ANY_FB||fb==k->stop_fb[i]))kind=0u;}
 dev[0]=io->superbank;dev[1]=io->interrupt_enabled;dev[2]=k->hw.in_interrupt;
 return digest(s,kind,3u,dev,3);
}
#endif
#if (RK_JOBS||RK_IDLE)&&RK_RUN
static uint32_t one(apollo_overlay_state *s,cm_native_io *io,const agc_routine_table *table){
 agc_hardware hw={0,1,0,0,0,0,0,0};agc_native_program p=program();agc_routine call;unsigned kind=3u;uint16_t dev[3];
 p.basic=table;p.basic_count=1;p.hardware=&hw;
 call=agc_transfer_resolve(&p,s,io->superbank);
 if(call)kind=call(&p,s,io)!=EXEC_OK?2u:s->words[5]==CM_EXEC_BAILOUT||s->words[5]==CM_EXEC_POODOO?1u:0u;
 dev[0]=io->superbank;dev[1]=io->interrupt_enabled;dev[2]=io->channel11;
 return digest(s,kind,3u,dev,3);
}
#endif
#if RK_JOBS&&RK_INPUT
void rk_jobs_input(apollo_overlay_state *s,rk_case *c){cm_exj_input(s,&c->io,c->n);rk_entry=s->words[5];}
#endif
#if RK_JOBS&&RK_RUN
uint32_t rk_jobs_run(apollo_overlay_state *s,rk_case *c){return one(s,&c->io,&cm_kernel_job_routines);}
#endif
#if RK_IDLE&&RK_INPUT
void rk_idle_input(apollo_overlay_state *s,rk_case *c){cm_exi_input(s,&c->io,c->n);rk_entry=s->words[5];}
#endif
#if RK_IDLE&&RK_RUN
uint32_t rk_idle_run(apollo_overlay_state *s,rk_case *c){return one(s,&c->io,&cm_kernel_idle_routines);}
#endif
#endif

#if RK_INPUT&&RK_RUN
#if RK_SERVICES
uint32_t rk_services_digest(apollo_overlay_state *s,uint16_t n){rk_case c;c.n=n;rk_services_input(s,&c);
#if RK_LM
 c.restart=&lm_restart_routines;
#endif
 return rk_services_run(s,&c);}
#endif
#if RK_JOBS
uint32_t rk_jobs_digest(apollo_overlay_state *s,uint16_t n){rk_case c;c.n=n;rk_jobs_input(s,&c);return rk_jobs_run(s,&c);}
#endif
#if RK_IDLE
uint32_t rk_idle_digest(apollo_overlay_state *s,uint16_t n){rk_case c;c.n=n;rk_idle_input(s,&c);return rk_idle_run(s,&c);}
#endif
#endif
