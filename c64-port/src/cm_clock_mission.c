/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission.h"
enum cm_service_status cm_clock_mission_identify(const cm_mission_loop *r,cm_mission_call *c){enum cm_service_status status;agc_word pc,bb;
 status=cm_mission_identify(r,c);if(status==CM_SERVICE_ADVANCED||status==CM_SERVICE_INVALID)return status;
 pc=r->image->words[5];bb=r->image->words[6];c->service=(cm_service_call){CM_SERVICE_APPLICATION,{pc,(agc_word)(bb|r->io->superbank)},{0,0},0};
 if(pc==03332u&&bb==050006u)c->group=CM_FLIGHT_CLOCK_ENTER;
 else if(pc==03555u&&bb==046006u)c->group=CM_FLIGHT_CLOCK_TRIG;
 else if(pc==03605u&&bb==046006u)c->group=CM_FLIGHT_CLOCK_RETURN;
 else if(pc==03342u&&bb==050006u)c->group=CM_FLIGHT_CLOCK_SELECT;
 else if((pc==02707u||pc==02715u||pc==02720u)&&r->image->words[4]==020000u)c->group=CM_FLIGHT_DISPLAY_CONTROL;
 else if(pc==03231u&&r->image->words[4]==02000u)c->group=CM_FLIGHT_IDLE;
 else return status;
 return CM_SERVICE_ADVANCED;}
enum cm_service_status cm_clock_mission_step(cm_mission_loop *r){cm_mission_call c;enum cm_service_status status;
 if(r==0||r->invoke==0)return CM_SERVICE_INVALID;
 if(r->in_callback)return CM_SERVICE_REENTER;
 status=cm_clock_mission_identify(r,&c);if(status!=CM_SERVICE_ADVANCED)return status;
 r->in_callback=1;status=r->invoke(&c,r->image,r->io,&r->irq,&r->transfer,r->user);r->in_callback=0;
 if(status==CM_SERVICE_ADVANCED){if(c.group==CM_MISSION_INTERRUPT)r->pending_t3=0;if(c.group==CM_MISSION_DANZIG)r->irq.pending_danzig=0;++r->completed_groups;}
 return status;}
