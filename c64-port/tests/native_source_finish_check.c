/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "native_source_finish_check.h"
#include "lm_finish_vectors.h"
#include "p66_native.h"
typedef struct {apollo_overlay_state *image;p66_bias_io io;agc_word channel;exec_native *runtime;exec_irq_queue *queue;uint8_t irq,depth,calls;enum exec_status status;} fixture;
static uint8_t enter(void *u){fixture *f=u;uint8_t old=f->irq;
#ifdef __mos__
 uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");old=(uint8_t)((p>>2)&1u);
#endif
 f->irq=1;++f->depth;return old;}
static void leave(void *u,uint8_t old){fixture*f=u;f->irq=old;--f->depth;
#ifdef __mos__
 if(old)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");
#endif
}
static enum exec_action unused(executive*s,void*u,exec_address*n){(void)s;(void)u;(void)n;return EXEC_FINISH;}
static enum exec_action source(executive*s,void*u,exec_address*n){fixture*f=u;exec_transfer next;uint8_t token=enter(f);exec_request request={01000,{06000,020007},0};++f->calls;f->status=lm_basic_finish(f->image,&f->io,&f->channel,&next);p66_native_import_executive(s,f->image);*n=next.address;
 if(f->status!=EXEC_OK&&f->status!=EXEC_IDLE)f->runtime->fault=f->status;
 if(exec_irq_post(f->queue,&request)!=EXEC_OK)f->runtime->fault=EXEC_OUT_OF_SCOPE;
 if(next.interpretive){f->runtime->interpreter_control.intbit15=f->image->words[0115];f->runtime->interpreter_control.edop=f->image->words[023];}
 leave(f,token);return next.interpretive?EXEC_DISPATCHED_INTERPRETER:EXEC_DISPATCHED_BASIC;}
uint16_t native_source_finish_check(uint8_t number,uint8_t irq,apollo_overlay_state *image,apollo_overlay_state *expected){fixture f;executive state;exec_native runtime;exec_irq_queue queue;exec_context_mapping maps[3];exec_exclusion guard={enter,leave,&f};p66_bias_io want;agc_word channel;exec_address next;enum exec_context_kind kind;uint16_t i,failures=0;uint8_t count=2;enum exec_status status;
 if(number>=64u||irq>1u)return 1;
 memset(&f,0,sizeof f);f.image=image;f.runtime=&runtime;f.queue=&queue;f.irq=irq;lm_finish_input(image,&f.io,&f.channel,lm_finish_numbers[number]);*expected=*image;want=f.io;channel=f.channel;
 for(i=lm_finish_offsets[number];i<lm_finish_offsets[number+1u];++i){uint16_t a=lm_finish_patch[i][0],v=lm_finish_patch[i][1];if(a<2048u)expected->words[a]=v;else if(a==2048u)want.superbank=v;else if(a==2049u)want.interrupt_enabled=v;else channel=v;}
 kind=expected->words[5]==06070u?EXEC_CONTEXT_INTERPRETIVE:EXEC_CONTEXT_BASIC;next.location=kind==EXEC_CONTEXT_INTERPRETIVE?expected->words[0164]:expected->words[5];next.bank=(agc_word)(expected->words[6]|want.superbank);
 maps[0]=(exec_context_mapping){{image->words[5],(agc_word)(image->words[6]|f.io.superbank)},EXEC_CONTEXT_BASIC,source};maps[1]=(exec_context_mapping){{06000,020007},EXEC_CONTEXT_BASIC,unused};
 if(lm_finish_results[number]==EXEC_OK)maps[count++]=(exec_context_mapping){next,kind,unused};
 p66_native_import_executive(&state,image);state.alarm=0;if(exec_native_context_init(&runtime,&state,maps,count,&f)!=EXEC_OK)return 1;exec_irq_init(&queue);if(exec_native_bind_irq(&runtime,&queue,&guard)!=EXEC_OK)return 1;runtime.current=maps[0].address;runtime.running=1;status=exec_native_step(&runtime);
 if(status!=lm_finish_results[number]||f.status!=status||f.calls!=1u||f.irq!=irq||f.depth||queue.read||queue.write!=1u||queue.overflow||queue.rejected)++failures;
 if(status==EXEC_OK){if(runtime.current.location!=next.location||runtime.current.bank!=next.bank||runtime.current_kind!=kind||runtime.running!=1u)++failures;}else if(runtime.running||runtime.current.location||runtime.current.bank||runtime.fault!=EXEC_OK)++failures;
 for(i=0;i<2048u;++i)if(image->words[i]!=expected->words[i])++failures;
 if(f.io.superbank!=want.superbank||f.io.interrupt_enabled!=want.interrupt_enabled||f.channel!=channel)++failures;
 return failures;
}
static enum exec_action invalid(executive*s,void*u,exec_address*n){exec_native*r=u;n->location=03353;n->bank=064007;switch(r->requested_priority){case 1:r->interpreter_control.edop=1;break;case 2:r->interpreter_control.intbit15=0;break;case 3:s->core[0][EXEC_PRIORITY]=0;break;case 4:n->location=1;break;case 5:n->location=0;n->bank=0;break;case 6:n->location=03354;break;case 7:r->current_kind=EXEC_CONTEXT_INTERPRETIVE;break;default:break;}r->requested_priority=0;return EXEC_DISPATCHED_INTERPRETER;}
uint16_t native_source_finish_guards(void){uint8_t mode;uint16_t failures=0;for(mode=1;mode<8u;++mode){executive s;exec_native r;exec_context_mapping maps[2]={{{02000,064007},EXEC_CONTEXT_BASIC,invalid},{{03353,064007},EXEC_CONTEXT_INTERPRETIVE,unused}};exec_init(&s);s.core[0][EXEC_PRIORITY]=01000;if(exec_native_context_init(&r,&s,maps,2,&r)!=EXEC_OK)return 1;r.current=maps[0].address;r.running=1;r.interpreter_control.intbit15=040000;
 /* Direct finish qualifies the adapter guards; whole real source transitions
  * above qualify the actual step, IRQ and exclusion intervals. */
 r.in_callback=1;r.requested_priority=mode;{exec_address next;enum exec_action action=invalid(&s,&r,&next);enum exec_status got=exec_native_finish(&r,action,next);if(got!=(mode==6u?EXEC_UNMAPPED:mode==7u?EXEC_INVALID_ACTION:EXEC_OUT_OF_SCOPE)||r.running)++failures;}}
 return failures;}
