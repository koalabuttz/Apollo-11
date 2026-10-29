/* SPDX-License-Identifier: MIT
 * Original STARTP66/VRTSTART/P66VERTA selects RODCOMP by fallthrough. There
 * is no source NEWJOB or asynchronous request poll at that transfer. Initial
 * pending jobs during STRTP66A remain excluded by the original DANZIG domain. */
#include "p66_direct_launch.h"
#include "p66_vertical_start.h"
#ifndef P66_DIRECT_LAUNCH_DISPATCH_ONLY
uint16_t p66_direct_launch_entry(apollo_overlay_state *image,p66_bias_io *io,uint16_t selector){
 if(selector==P66_DIRECT_LAUNCH_SERVICE)return (uint16_t)p66_launch(image,io);
 if(selector==P66_DIRECT_VERTICAL_SERVICE)return (uint16_t)p66_vertical_start(image,io);
 return P66_LAUNCH_OUT_OF_SCOPE;
}
#endif
enum exec_action p66_direct_launch_callback(executive *state,void *user,exec_address *resume){
 p66_basic_binding *b=user;uint16_t bank,selector;uint8_t token;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);
 p66_native_export_executive(b->arithmetic.image,state);
 bank=(uint16_t)(b->arithmetic.runtime->current.bank&076007u);
 selector=b->arithmetic.runtime->current.location==02477 ? P66_DIRECT_LAUNCH_SERVICE : P66_DIRECT_VERTICAL_SERVICE;
 b->arithmetic.image->words[3]=(uint16_t)((bank&7u)<<8);b->arithmetic.image->words[4]=(uint16_t)(bank&076000u);b->arithmetic.image->words[6]=bank;b->arithmetic.image->words[5]=b->arithmetic.runtime->current.location;
 b->control.superbank=(uint16_t)(b->arithmetic.runtime->current.bank&0160u);
 if(b->invoke_basic!=0)b->status=b->invoke_basic(b->basic_modules,b->arithmetic.image,&b->control,selector);
#ifndef P66_DIRECT_LAUNCH_DISPATCH_ONLY
 else b->status=p66_direct_launch_entry(b->arithmetic.image,&b->control,selector);
#else
 else b->status=P66_LAUNCH_OUT_OF_SCOPE;
#endif
 p66_native_import_executive(state,b->arithmetic.image);
 resume->location=b->arithmetic.image->words[5];resume->bank=(uint16_t)(b->arithmetic.image->words[6]|b->control.superbank);
 if(b->status!=0)b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;
 if(b->status==0 && b->arithmetic.observe!=0)b->arithmetic.observe(b->arithmetic.observer,b->arithmetic.image,resume->location);
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);
 return EXEC_DISPATCHED_BASIC;
}
const exec_context_mapping p66_direct_launch_mappings[P66_DIRECT_LAUNCH_MAPPING_COUNT]={
 {{02477,062007},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062027},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062047},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062067},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062007},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062027},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062047},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062067},EXEC_CONTEXT_BASIC,p66_direct_launch_callback}
};
