/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_INTERFACE_APPLY_H
#define CM_POWERED_INTERFACE_APPLY_H
#include "cm_powered_interface.h"
#include "cm_powered_trig.h"
#include "cm_powered_axis.h"
/* Test-only source call composition. No source jobcheck occurs in this call. */
static uint8_t cm_powered_interface_apply(apollo_overlay_state *m,agc_word *irq,uint8_t selector){uint8_t step;
if(cm_powered_interface_start(m,selector)!=CM_POWERED_INTERFACE_OK)return 0;
if(selector<6u){if(cm_powered_cdu_start(m,(uint8_t)((selector&1u)==0))!=CM_POWERED_TRIG_OK)return 0;
for(step=1;step<10u;++step){enum cm_powered_trig_status result=step%3u==1u?cm_powered_cdu_cosine(m):step%3u==2u?cm_powered_cdu_sine(m):cm_powered_cdu_next(m);if(result!=CM_POWERED_TRIG_OK)return 0;}
if(selector>=2u&&cm_powered_interface_axis(m)!=CM_POWERED_INTERFACE_OK)return 0;}
if(selector>=2u&&(cm_powered_axis(m)!=CM_POWERED_AXIS_OK||cm_powered_axis_return(m)!=CM_POWERED_AXIS_OK))return 0;
if(cm_powered_interface_reentry(m,irq)!=CM_POWERED_INTERFACE_OK)return 0;
if(selector>=2u&&cm_powered_interface_load(m)!=CM_POWERED_INTERFACE_OK)return 0;
return (uint8_t)(cm_powered_interface_return(m)==CM_POWERED_INTERFACE_OK);}
#endif
