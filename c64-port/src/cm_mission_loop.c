/* SPDX-License-Identifier: MIT */
#include "cm_mission_loop.h"
enum cm_service_status cm_mission_identify(const cm_mission_loop *r,cm_mission_call *c){enum cm_service_status status;
 if(r==0||r->image==0||r->io==0||c==0||r->pending_t3>1u||r->irq.in_isr>1u||r->irq.pending_danzig>1u)return CM_SERVICE_INVALID;
 if(r->pending_t3&&!r->irq.in_isr&&!r->irq.pending_danzig&&r->image->words[5]==06030u){c->group=CM_MISSION_INTERRUPT;return CM_SERVICE_ADVANCED;}
 if(r->image->words[5]==05341u&&r->irq.in_isr){c->group=CM_MISSION_RESUME;return CM_SERVICE_ADVANCED;}
 if(r->image->words[5]==06030u&&!r->irq.in_isr){c->group=CM_MISSION_DANZIG;return CM_SERVICE_ADVANCED;}
 if(r->image->words[5]==05217u&&!r->irq.in_isr){c->group=CM_MISSION_ENDJOB;return CM_SERVICE_ADVANCED;}
 status=cm_service_identify(r->image,r->io,&c->service);c->group=(unsigned)c->service.group;return status;}
enum cm_service_status cm_mission_step(cm_mission_loop *r){cm_mission_call c;enum cm_service_status status;
 if(r==0||r->invoke==0)return CM_SERVICE_INVALID;
 if(r->in_callback)return CM_SERVICE_REENTER;
 status=cm_mission_identify(r,&c);if(status!=CM_SERVICE_ADVANCED)return status;
 r->in_callback=1;status=r->invoke(&c,r->image,r->io,&r->irq,&r->transfer,r->user);r->in_callback=0;
 if(status==CM_SERVICE_ADVANCED){if(c.group==CM_MISSION_INTERRUPT)r->pending_t3=0;if(c.group==CM_MISSION_DANZIG)r->irq.pending_danzig=0;++r->completed_groups;}
 return status;}
