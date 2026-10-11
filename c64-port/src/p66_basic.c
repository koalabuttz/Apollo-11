/* SPDX-License-Identifier: MIT */
#include "p66_basic.h"
#include "p66_launch.h"
#include "p66_mode_display.h"
#include "p66_startup.h"
#include "dsky_relay.h"
#include <stddef.h>
typedef char arithmetic_binding_must_be_first[(offsetof(p66_basic_binding,arithmetic)==0)?1:-1];
#ifndef P66_BASIC_DISPATCH_ONLY
uint16_t p66_basic_entry(apollo_overlay_state *m,p66_bias_io *io,uint16_t selector){
 switch(selector){
#ifndef P66_BASIC_NO_LAUNCH
 case P66_BASIC_LAUNCH:return (uint16_t)p66_launch(m,io);
#endif
 case P66_BASIC_MODE_DISPLAY:return (uint16_t)p66_mode_display(m,io);
 default:return P66_BIAS_OUT_OF_SCOPE;
 }
}
#endif
static int valid(p66_basic_binding *b){return b!=0 && b->arithmetic.image!=0 && b->arithmetic.runtime!=0 && b->arithmetic.producers!=0 && b->arithmetic.producers->enter!=0 && b->arithmetic.producers->leave!=0;}
static void wait_view(waitlist *s,uint16_t *w,uint8_t import){uint8_t i;
 if(import){s->time3=w[026];s->ruptagn=w[0734];}else{w[026]=s->time3;w[0734]=s->ruptagn;}
 for(i=0;i<8;++i){if(import)s->interval[i]=w[01400+i];else w[01400+i]=s->interval[i];}
 for(i=0;i<9;++i){if(import){s->task[i].location=w[01410+2u*i];s->task[i].bank=w[01411+2u*i];}else{w[01410+2u*i]=s->task[i].location;w[01411+2u*i]=s->task[i].bank;}}
}
static enum exec_action basic(executive *s,void *user,exec_address *resume){
 p66_basic_binding *b=user;uint16_t selector;uint8_t token=b->arithmetic.producers->enter(b->arithmetic.producers->user);
 selector=b->arithmetic.runtime->current.location==02477 ? P66_BASIC_LAUNCH : P66_BASIC_MODE_DISPLAY;
 p66_native_export_executive(b->arithmetic.image,s);
 /* Native dispatch metadata selects the actual physical basic entry. */
 {uint16_t bank=(uint16_t)(b->arithmetic.runtime->current.bank&076007u);uint16_t *w=b->arithmetic.image->words;w[3]=(uint16_t)((bank&7u)<<8);w[4]=(uint16_t)(bank&076000u);w[6]=bank;w[5]=b->arithmetic.runtime->current.location;b->control.superbank=(uint16_t)(b->arithmetic.runtime->current.bank&0160u);}
 if(b->invoke_basic!=0)b->status=b->invoke_basic(b->basic_modules,b->arithmetic.image,&b->control,selector);
#ifndef P66_BASIC_DISPATCH_ONLY
 else b->status=p66_basic_entry(b->arithmetic.image,&b->control,selector);
#else
 else b->status=P66_BIAS_OUT_OF_SCOPE;
#endif
 p66_native_import_executive(s,b->arithmetic.image);
 resume->location=b->arithmetic.image->words[5];resume->bank=(uint16_t)(b->arithmetic.image->words[6]|b->control.superbank);
 if(b->status!=0)b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;
 if(b->status==0 && b->arithmetic.observe!=0)b->arithmetic.observe(b->arithmetic.observer,b->arithmetic.image,resume->location);
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);
 return selector==P66_BASIC_MODE_DISPLAY ? EXEC_FINISH : EXEC_YIELD;
}
const exec_context_mapping p66_basic_mappings[P66_BASIC_MAPPING_COUNT]={
 {{02477,062007},EXEC_CONTEXT_BASIC,basic},{{02477,062067},EXEC_CONTEXT_BASIC,basic},{{03534,060102},EXEC_CONTEXT_BASIC,basic}
};
static enum wl_status rodtask(wl_native *r,void *user){
 p66_basic_binding *b=user;p66_task_io io;executive *s=b->arithmetic.runtime->state;uint16_t *w=b->arithmetic.image->words;enum exec_status status;exec_address entry={P66_RODCOMP_LOCATION,P66_RODCOMP_BBCON};
 if(!exec_native_is_mapped(b->arithmetic.runtime,entry,EXEC_CONTEXT_BASIC))return WL_UNMAPPED;
 wait_view(r->state,w,0);p66_native_import_executive(s,b->arithmetic.image);
 io.a=w[0];io.l=w[1];io.q=w[2];io.bbank=(uint16_t)(r->current.bank&076007u);io.superbank=(uint16_t)(r->current.bank&0160u);io.interrupt_enabled=0;io.exectem1=w[061];io.exectem2=w[062];
 status=p66_rodtask(s,&io);b->status=(uint16_t)status;
 if(status==EXEC_OUT_OF_SCOPE || status==EXEC_BAILOUT_PENDING)return WL_OUT_OF_SCOPE;
 p66_native_export_executive(b->arithmetic.image,s);
 w[0]=io.a;w[1]=io.l;w[2]=io.q;w[3]=(uint16_t)((io.bbank&7u)<<8);w[4]=(uint16_t)(io.bbank&076000u);w[6]=io.bbank;w[061]=io.exectem1;w[062]=io.exectem2;w[5]=status==EXEC_OK ? 05261 : 05716;w[7]=0;
 b->control.superbank=io.superbank;b->control.interrupt_enabled=io.interrupt_enabled;
 if(b->arithmetic.observe!=0)b->arithmetic.observe(b->arithmetic.observer,b->arithmetic.image,w[5]);
 if(status!=EXEC_OK){b->arithmetic.runtime->fault=status;return WL_ALARM_PENDING;}return WL_OK;
}
const wl_mapping p66_basic_task_mappings[P66_BASIC_TASK_MAPPING_COUNT]={{{03263,064007},rodtask},{{03263,064067},rodtask}};
enum wl_status p66_basic_task_init(p66_basic_binding *b,wl_native *r,waitlist *s){
 if(!valid(b) || r==0 || s==0)return WL_OUT_OF_SCOPE;
 b->tasks=r;s->alarm=0;wait_view(s,b->arithmetic.image->words,1);
 return wl_native_init(r,s,p66_basic_task_mappings,P66_BASIC_TASK_MAPPING_COUNT,b,b->arithmetic.producers);
}
enum wl_status p66_basic_schedule(p66_basic_binding *b,agc_word delay,exec_address task){enum wl_status status;uint8_t token;
 if(!valid(b) || b->tasks==0)return WL_OUT_OF_SCOPE;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);wait_view(b->tasks->state,b->arithmetic.image->words,1);status=wl_native_schedule(b->tasks,delay,task);wait_view(b->tasks->state,b->arithmetic.image->words,0);b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum wl_status p66_basic_advance(p66_basic_binding *b,uint16_t ticks){enum wl_status status;uint8_t token;
 if(!valid(b) || b->tasks==0)return WL_OUT_OF_SCOPE;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);wait_view(b->tasks->state,b->arithmetic.image->words,1);status=wl_native_advance(b->tasks,ticks);wait_view(b->tasks->state,b->arithmetic.image->words,0);if(status==WL_OK && b->display!=0)dsky_display_advance(b->display,ticks);b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum wl_status p66_basic_task_step(p66_basic_binding *b){enum wl_status status;uint8_t token;
 if(!valid(b) || b->tasks==0)return WL_OUT_OF_SCOPE;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);wait_view(b->tasks->state,b->arithmetic.image->words,1);status=wl_native_step(b->tasks);wait_view(b->tasks->state,b->arithmetic.image->words,0);b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum exec_status p66_basic_step(p66_basic_binding *b){enum exec_status status;uint8_t token;enum exec_context_kind previous;exec_native *r;
 if(!valid(b))return EXEC_OUT_OF_SCOPE;
 r=b->arithmetic.runtime;previous=r->current_kind;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);p66_native_import_executive(r->state,b->arithmetic.image);b->arithmetic.producers->leave(b->arithmetic.producers->user,token);
 status=exec_native_step(r);
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);p66_native_export_executive(b->arithmetic.image,r->state);
 if(status==EXEC_OK){uint16_t bank=(uint16_t)(r->current.bank&076007u);b->arithmetic.image->words[6]=bank;b->arithmetic.image->words[3]=(uint16_t)((bank&7u)<<8);b->arithmetic.image->words[4]=(uint16_t)(bank&076000u);b->control.superbank=(uint16_t)(r->current.bank&0160u);}
 /* WDRET aliases INTBIT15. Basic decimal jobs must retain real WDRET stores. */
 if(previous==EXEC_CONTEXT_INTERPRETIVE || (status==EXEC_OK && r->current_kind==EXEC_CONTEXT_INTERPRETIVE)){b->arithmetic.image->words[0115]=r->interpreter_control.intbit15;b->arithmetic.image->words[023]=r->interpreter_control.edop;}
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum p66_bias_status p66_basic_relay(p66_basic_binding *b){dsky_relay_io io;enum p66_bias_status status;uint8_t token;
 if(!valid(b) || b->display==0)return P66_BIAS_OUT_OF_SCOPE;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);io.control=b->control;io.wrote_channel10=0;io.channel10=0;status=dsky_relay_drain(b->arithmetic.image,&io);if(status==P66_BIAS_OK){b->control=io.control;if(io.wrote_channel10!=0)dsky_display_output(b->display,010,io.channel10);}b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
