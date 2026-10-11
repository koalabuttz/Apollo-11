/* SPDX-License-Identifier: MIT */
#include "cm_service_loop.h"
static int banks(const apollo_overlay_state *s,const cm_native_io *io){return s!=0&&io!=0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&io->interrupt_enabled<=1u&&(io->superbank&~0160u)==0;}
enum cm_service_status cm_service_identify(const apollo_overlay_state *s,const cm_native_io *io,cm_service_call *c){agc_word pc,bb,q;
 if(c==0||!banks(s,io))return CM_SERVICE_INVALID;
 pc=s->words[5];bb=s->words[6];q=s->words[2];c->entry.location=pc;c->entry.bank=(agc_word)(bb|io->superbank);c->argument=(exec_address){0,0};c->literal=0;
 if(pc==05341u){c->group=CM_SERVICE_SOURCE_RESUME;return CM_SERVICE_RESUME;}
 if(pc==05324u)c->group=CM_SERVICE_TASK_FINISH;
 else if(pc==03423u&&bb==02003u)c->group=CM_SERVICE_T3_BEGIN;
 else if(pc==03207u&&bb==032006u)c->group=CM_SERVICE_SETJTAG;
 else if(bb==050007u&&(pc==03260u||pc==03301u||pc==03315u||pc==03324u))c->group=pc==03260u?CM_SERVICE_CLOCK_ENTRY:pc==03301u?CM_SERVICE_CLOCK_RETURN:CM_SERVICE_CLOCK_FINISH;
 else if(bb==050007u&&(pc==02574u||pc==02600u||pc==02611u))c->group=pc==02574u?CM_SERVICE_TIG5_ENTRY:pc==02600u?CM_SERVICE_TIG5_RETURN:CM_SERVICE_TIG5_FINISH;
 else if(bb==004003u&&(pc==05306u||pc==05323u||pc==07721u))c->group=pc==05306u?CM_SERVICE_SVCT3_ENTRY:pc==05323u?CM_SERVICE_SVCT3_RETURN:CM_SERVICE_SVCT3_RECHECK;
 else if(pc==05267u&&q==07720u&&bb==004003u){c->group=CM_SERVICE_FIXDELAY;c->literal=500;}
 else if(pc==05245u){c->group=CM_SERVICE_WAITLIST;
  if(bb==050007u&&q==03277u)c->argument=(exec_address){03260,050067};
  else if(bb==050007u&&q==02576u)c->argument=(exec_address){02612,050066};
  else if(bb==02003u&&q==03512u)c->argument=(exec_address){03465,02063};
  else if(bb==02003u&&q==03503u)c->argument=(exec_address){03515,02063};
  else if(bb==02003u&&q==0705u)c->argument=(exec_address){s->words[0705],s->words[0706]};
  else return CM_SERVICE_UNMAPPED;
 }
 else if(pc==05134u||pc==05147u){c->group=pc==05134u?CM_SERVICE_NOVAC:CM_SERVICE_FINDVAC;
  if(bb==050007u&&q==03313u&&pc==05134u)c->argument=(exec_address){03332,050066};
  else if(bb==050007u&&q==03322u&&pc==05147u)c->argument=(exec_address){03325,050066};
  else if(bb==050007u&&q==02607u&&pc==05147u)c->argument=(exec_address){02427,034067};
  else if(bb==004003u&&q==05321u&&pc==05134u)c->argument=(exec_address){03542,014063};
  else if(bb==02003u&&q==03540u&&pc==05147u)c->argument=(exec_address){03261,020063};
  else if(bb==02003u&&q==0705u)c->argument=(exec_address){s->words[0705],s->words[0706]};
  else return CM_SERVICE_UNMAPPED;
 }
 else if(bb==02003u&&(pc==03465u||pc==03505u||pc==03514u||pc==03515u))c->group=pc==03465u?CM_SERVICE_LONG_CYCLE:pc==03515u?CM_SERVICE_LONG_TARGET:CM_SERVICE_LONG_RETURN;
 else if(pc==05342u&&bb==02003u&&q==0705u){c->group=CM_SERVICE_LONG_ENTER;c->argument=(exec_address){s->words[0705],s->words[0706]};}
 else if(bb==02003u&&(pc==03520u||pc==03755u||pc==03533u))c->group=pc==03520u?CM_SERVICE_RESTART_GROUP:pc==03755u?CM_SERVICE_RESTART_PART2:CM_SERVICE_RESTART_TYPEB;
 else if(pc==0704u&&bb==02003u&&(q==03637u||q==03643u||q==03614u||q==03726u)){
  c->group=CM_SERVICE_RESTART_CALL;c->literal=q==03637u?05147:q==03643u?05134:q==03614u?05245:05342;
 }
 else if(s->words[4]==020000u&&(pc==03261u||pc==03267u))c->group=pc==03261u?CM_SERVICE_INIT_DISPLAY:CM_SERVICE_INIT_DISPLAY_RETURN;
 else{c->group=CM_SERVICE_APPLICATION;return CM_SERVICE_UNMAPPED;}
 return CM_SERVICE_ADVANCED;
}
enum cm_service_status cm_service_step(cm_service_loop *r){cm_service_call c;enum cm_service_status status;
 if(r==0||r->invoke==0)return CM_SERVICE_INVALID;
 if(r->in_callback)return CM_SERVICE_REENTER;
 status=cm_service_identify(r->image,r->io,&c);if(status!=CM_SERVICE_ADVANCED)return status;
 r->in_callback=1;status=r->invoke(&c,r->image,r->io,r->user);r->in_callback=0;
 if(status==CM_SERVICE_ADVANCED)++r->completed_groups;
 return status;
}
