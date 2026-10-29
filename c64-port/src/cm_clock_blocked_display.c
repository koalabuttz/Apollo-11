/* SPDX-License-Identifier: MIT */
#include "cm_clock_blocked_display.h"
#include "cm_normal_display.h"
#include "cm_display_rest.h"
#include "cm_basic_sleep.h"
#include "dsky_nv_busy.h"
enum cm_service_status cm_clock_blocked_display_step(cm_mission_loop *r,uint8_t phase)
{
    enum cm_service_status result=CM_SERVICE_FAULT;enum exec_status status;
    enum p66_bias_status admission;enum dsky_nv_route route;enum dsky_nv_busy_route busy;
    p66_bias_io io;
    if(r==0||r->image==0||r->io==0||phase<7u||phase>11u||r->image->words[0371]!=01450u)return CM_SERVICE_INVALID;
    if(r->in_callback)return CM_SERVICE_REENTER;
    r->in_callback=1;
    if(phase==7u)result=cm_normal_display_caller(r->image,r->io,0)?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
    else if(phase==8u){io.superbank=r->io->superbank;io.interrupt_enabled=r->io->interrupt_enabled;admission=dsky_nv_admit(r->image,&io,DSKY_NV_CM,DSKY_NV_NVMONOPT,&route);if(admission==P66_BIAS_OK){r->io->superbank=io.superbank;r->io->interrupt_enabled=io.interrupt_enabled;result=CM_SERVICE_ADVANCED;}}
    else if(phase==9u){status=cm_display_rest(r->image,r->io);if((unsigned)status==(unsigned)EXEC_OK)result=CM_SERVICE_ADVANCED;}
    else if(phase==10u){io.superbank=r->io->superbank;io.interrupt_enabled=r->io->interrupt_enabled;admission=dsky_nv_busy(r->image,&io,&r->io->channel11,DSKY_NV_CM,DSKY_NV_BUSY,&busy);if(admission==P66_BIAS_OK){r->io->superbank=io.superbank;r->io->interrupt_enabled=io.interrupt_enabled;result=CM_SERVICE_ADVANCED;}}
    else{status=cm_basic_sleep(r->image,r->io,&r->transfer);if((unsigned)status==(unsigned)EXEC_OK||(unsigned)status==(unsigned)EXEC_IDLE)result=CM_SERVICE_ADVANCED;}
    r->in_callback=0;if(result==CM_SERVICE_ADVANCED)++r->completed_groups;return result;
}
