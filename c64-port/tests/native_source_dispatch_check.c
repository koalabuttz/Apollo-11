/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "native_source_dispatch_check.h"
#ifdef NATIVE_SOURCE_DISPATCH_SMALL_VECTORS
#include "native_source_dispatch_priority_vectors.h"
#include "native_source_dispatch_sleep_vectors.h"
#else
#include "lm_priority_vectors.h"
#include "lm_sleep_vectors.h"
#endif
#include "p66_native.h"
typedef struct {
 apollo_overlay_state *image;p66_bias_io io;agc_word channel11;
 exec_native *runtime;exec_irq_queue *queue;uint8_t service,irq,depth,calls;
 enum exec_status source_status;
} fixture;
static uint8_t enter(void *u){fixture *f=u;uint8_t old=f->irq;
#ifdef __mos__
 uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");old=(uint8_t)((p>>2)&1u);
#endif
 f->irq=1;++f->depth;return old;}
static void leave(void *u,uint8_t previous){fixture *f=u;f->irq=previous;--f->depth;
#ifdef __mos__
 if(previous)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");
#endif
}
static enum exec_action unused(executive *s,void *u,exec_address *next){(void)s;(void)u;(void)next;return EXEC_FINISH;}
static enum exec_action source(executive *s,void *u,exec_address *next){
 fixture *f=u;uint8_t token=enter(f);exec_request request={01000,{06000,020007},0};
 ++f->calls;
 f->source_status=f->service?lm_basic_sleep(f->image,&f->io,&f->channel11,next):
  lm_basic_priority_change(f->image,&f->io,next);
 p66_native_import_executive(s,f->image);
 if(f->source_status!=EXEC_OK&&f->source_status!=EXEC_IDLE)f->runtime->fault=f->source_status;
 if(exec_irq_post(f->queue,&request)!=EXEC_OK)f->runtime->fault=EXEC_OUT_OF_SCOPE;
 leave(f,token);return EXEC_DISPATCHED_BASIC;
}
uint16_t native_source_dispatch_check(uint8_t service,uint8_t number,uint8_t irq,
 apollo_overlay_state *image,apollo_overlay_state *before){
 fixture f;executive state;exec_native runtime;exec_irq_queue queue;
 exec_context_mapping maps[3];exec_exclusion guard={enter,leave,&f};
 uint16_t i,patch,end,expected_pc,expected_bb,expected_super,failures=0;
 uint8_t count=2;enum exec_status expected,status;p66_bias_io initial;agc_word initial_channel;
 if(service>1u||number>=NATIVE_SOURCE_DISPATCH_CASES||irq>1u)return 1;
 memset(&f,0,sizeof f);f.image=image;f.runtime=&runtime;f.queue=&queue;f.service=service;f.irq=irq;
 if(service)lm_sleep_input(image,&f.io,&f.channel11,lm_sleep_numbers[number]);
 else lm_priority_input(image,&f.io,lm_priority_numbers[number]);
 *before=*image;initial=f.io;initial_channel=f.channel11;
 patch=service?lm_sleep_offsets[number]:lm_priority_offsets[number];
 end=service?lm_sleep_offsets[number+1u]:lm_priority_offsets[number+1u];
 expected=service?(enum exec_status)lm_sleep_results[number]:(enum exec_status)lm_priority_results[number];
 expected_pc=image->words[5];expected_bb=image->words[6];expected_super=f.io.superbank;
 for(i=patch;i<end;++i){uint16_t address=service?lm_sleep_patch[i][0]:lm_priority_patch[i][0];uint16_t value=service?lm_sleep_patch[i][1]:lm_priority_patch[i][1];
  if(address==5u)expected_pc=value;else if(address==6u)expected_bb=value;else if(address==2048u)expected_super=value;
 }
 maps[0]=(exec_context_mapping){{image->words[5],(agc_word)(image->words[6]|f.io.superbank)},EXEC_CONTEXT_BASIC,source};
 maps[1]=(exec_context_mapping){{06000,020007},EXEC_CONTEXT_BASIC,unused};
 if(expected==EXEC_OK && (maps[0].address.location!=expected_pc||maps[0].address.bank!=(expected_bb|expected_super)))
  maps[count++]=(exec_context_mapping){{expected_pc,(agc_word)(expected_bb|expected_super)},EXEC_CONTEXT_BASIC,unused};
 p66_native_import_executive(&state,image);state.alarm=0;
 if(exec_native_context_init(&runtime,&state,maps,count,&f)!=EXEC_OK)return 1;
 exec_irq_init(&queue);if(exec_native_bind_irq(&runtime,&queue,&guard)!=EXEC_OK)return 1;
 runtime.current=maps[0].address;runtime.current_kind=EXEC_CONTEXT_BASIC;runtime.running=1;
 status=exec_native_step(&runtime);if(status!=expected||f.source_status!=expected)++failures;
 if(f.calls!=1u||f.irq!=irq||f.depth!=0u||queue.read!=0u||queue.write!=1u||queue.overflow||queue.rejected)++failures;
 if(expected==EXEC_OK){if(runtime.current.location!=expected_pc||runtime.current.bank!=(expected_bb|expected_super)||runtime.running!=1u||runtime.current_kind!=EXEC_CONTEXT_BASIC)++failures;}
 else if(runtime.running||runtime.current.location||runtime.current.bank||runtime.fault!=EXEC_OK)++failures;
 for(i=0;i<2048u+(service?3u:2u);++i){uint16_t actual,value;
  value=i<2048u?before->words[i]:i==2048u?initial.superbank:i==2049u?initial.interrupt_enabled:0;
  if(i==2050u)value=initial_channel;
  if(patch<end && (service?lm_sleep_patch[patch][0]:lm_priority_patch[patch][0])==i)
   value=service?lm_sleep_patch[patch++][1]:lm_priority_patch[patch++][1];
  actual=i<2048u?image->words[i]:i==2048u?f.io.superbank:i==2049u?f.io.interrupt_enabled:f.channel11;
  if(actual!=value)++failures;
 }
 if(patch!=end)++failures;
 return failures;
}
static enum exec_action invalid(executive *s,void *u,exec_address *next){
 uint8_t mode=*(uint8_t *)u;
 if(mode==0){next->location=0;next->bank=0;}
 else if(mode==1){next->location=03000;next->bank=020007;}
 else if(mode==2){next->location=02000;next->bank=077777;}
 else {s->core[0][EXEC_PRIORITY]=0;next->location=02000;next->bank=020007;}
 return EXEC_DISPATCHED_BASIC;
}
uint16_t native_source_dispatch_guards(void){uint16_t failures=0;uint8_t mode;
 for(mode=0;mode<4u;++mode){executive state;exec_native runtime;exec_context_mapping map={{02000,020007},EXEC_CONTEXT_BASIC,invalid};
  exec_init(&state);state.core[0][EXEC_PRIORITY]=01000;
  if(exec_native_context_init(&runtime,&state,&map,1,&mode)!=EXEC_OK)return 1;
  runtime.running=1;runtime.current=map.address;
  if(exec_native_step(&runtime)!=(mode==1u?EXEC_UNMAPPED:EXEC_OUT_OF_SCOPE)||runtime.running)++failures;
 }
 return failures;
}
