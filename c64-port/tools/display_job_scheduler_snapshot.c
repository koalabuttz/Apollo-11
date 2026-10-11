/* SPDX-License-Identifier: MIT */
#include "native_overlay_dispatch.h"
static enum exec_status scheduler_context_init(exec_native *,executive *,const exec_context_mapping *,uint8_t,void *);
#define exec_native_context_init scheduler_context_init
#if defined(DISPLAY_JOB_FINAL)
#include "display_job_final_snapshot.c"
#elif defined(DISPLAY_JOB_LIFECYCLE)
#include "display_job_lifecycle_snapshot.c"
#else
#include "display_job_snapshot.c"
#endif
#undef exec_native_context_init
static exec_irq_queue requests;
static uint16_t scheduler(void *user,exec_overlay_frame *frame,uint16_t stage){(void)user;if(depth!=1u)return 0xffff;if(stage==EXEC_OVERLAY_PREPARE)return(uint16_t)exec_native_prepare(frame->runtime,&frame->call);if(stage==EXEC_OVERLAY_FINISH)return(uint16_t)exec_native_finish(frame->runtime,frame->action,frame->resume);return 0xffff;}
static const exec_overlay_dispatch service={scheduler,0};
static enum exec_status scheduler_context_init(exec_native *r,executive *e,const exec_context_mapping *m,uint8_t count,void *user){enum exec_status status=exec_native_context_init(r,e,m,count,user);exec_irq_init(&requests);if(status==EXEC_OK)status=exec_native_bind_irq(r,&requests,&producers);if(status==EXEC_OK)status=exec_overlay_bind(r,&service);return status;}
