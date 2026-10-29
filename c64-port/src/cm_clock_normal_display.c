/* SPDX-License-Identifier: MIT */
#include "cm_clock_normal_display.h"
#include "cm_normal_display.h"
#include "cm_basic_finish.h"
enum cm_service_status cm_clock_normal_display_step(cm_mission_loop *r,uint8_t phase)
{
    enum cm_service_status result;
    enum exec_status status;
    if(r==0||r->image==0||r->io==0||phase<7u||phase>29u||r->image->words[0371]!=01450u)
        return CM_SERVICE_INVALID;
    if(r->in_callback)return CM_SERVICE_REENTER;
    r->in_callback=1;
    if(phase<29u)
        result=cm_normal_display_step(r->image,r->io,phase)?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
    else {
        status=cm_basic_finish(r->image,r->io,&r->transfer);
        result=(unsigned)status==(unsigned)EXEC_OK||(unsigned)status==(unsigned)EXEC_IDLE?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;
    }
    r->in_callback=0;
    if(result==CM_SERVICE_ADVANCED)++r->completed_groups;
    return result;
}
