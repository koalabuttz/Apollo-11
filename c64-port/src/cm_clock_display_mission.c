/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_mission.h"
#include "cm_makeplay.h"
#include "cm_makeplay_sleep.h"
#include "cm_priority_complete.h"
#include "cm_basic_wake.h"
#include "cm_basic_sleep.h"
enum cm_service_status cm_clock_display_identify(const cm_mission_loop *r,cm_mission_call *c){enum cm_service_status st;agc_word pc;
 st=cm_clock_mission_identify(r,c);if(st==CM_SERVICE_ADVANCED||st==CM_SERVICE_INVALID)return st;
 pc=r->image->words[5];
 if(pc==05210)c->group=CM_CLOCK_DISPLAY_PRIORITY;
 else if(pc==05201)c->group=CM_CLOCK_DISPLAY_WAKE;
 else if(pc==05175)c->group=CM_CLOCK_DISPLAY_SLEEP;
 else if(r->image->words[4]==020000){switch(pc){case 02770:c->group=CM_CLOCK_DISPLAY_MAKEPLAY;break;case 02775:c->group=CM_CLOCK_DISPLAY_SELECT;break;case 03036:c->group=CM_CLOCK_DISPLAY_WAKE_RETURN;break;case 03176:c->group=CM_CLOCK_DISPLAY_SLEEP_ENTER;break;case 03212:c->group=CM_CLOCK_DISPLAY_SLEEP_RETURN;break;case 03037:c->group=CM_CLOCK_DISPLAY_SLEEP_RESUME;break;default:return st;}}
 else return st;
 c->service=(cm_service_call){CM_SERVICE_APPLICATION,{pc,(agc_word)(r->image->words[6]|r->io->superbank)},{0,0},0};return CM_SERVICE_ADVANCED;}
enum cm_service_status cm_clock_display_step(cm_mission_loop *r){cm_mission_call c;enum cm_service_status st;
 if(!r||!r->invoke)return CM_SERVICE_INVALID;
 if(r->in_callback)return CM_SERVICE_REENTER;
 st=cm_clock_display_identify(r,&c);if(st!=CM_SERVICE_ADVANCED)return st;
 r->in_callback=1;st=r->invoke(&c,r->image,r->io,&r->irq,&r->transfer,r->user);r->in_callback=0;
 if(st==CM_SERVICE_ADVANCED){if(c.group==CM_MISSION_INTERRUPT)r->pending_t3=0;if(c.group==CM_MISSION_DANZIG)r->irq.pending_danzig=0;++r->completed_groups;}return st;}
