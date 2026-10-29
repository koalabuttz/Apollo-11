/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_INTERFACE_APPLY_H
#define LM_POWERED_INTERFACE_APPLY_H
#include "lm_powered_interface.h"
#include "lm_powered_trig.h"
#include "lm_powered_axis.h"
/* Test-only source call composition. No source jobcheck occurs in this call. */
static uint8_t lm_powered_interface_apply(apollo_overlay_state *m,agc_word *irq,uint8_t selector){uint8_t step;
if(lm_powered_interface_start(m,selector)!=LM_POWERED_INTERFACE_OK)return 0;
if(selector<6u){if(lm_powered_cdu_start(m,(uint8_t)((selector&1u)==0))!=LM_POWERED_TRIG_OK)return 0;
for(step=1;step<10u;++step){enum lm_powered_trig_status result=step%3u==1u?lm_powered_cdu_cosine(m):step%3u==2u?lm_powered_cdu_sine(m):lm_powered_cdu_next(m);if(result!=LM_POWERED_TRIG_OK)return 0;}
if(selector>=2u&&lm_powered_interface_axis(m)!=LM_POWERED_INTERFACE_OK)return 0;}
if(selector>=2u&&(lm_powered_axis(m)!=LM_POWERED_AXIS_OK||lm_powered_axis_return(m)!=LM_POWERED_AXIS_OK))return 0;
if(lm_powered_interface_reentry(m,irq)!=LM_POWERED_INTERFACE_OK)return 0;
if(selector>=2u&&lm_powered_interface_load(m)!=LM_POWERED_INTERFACE_OK)return 0;
return (uint8_t)(lm_powered_interface_return(m)==LM_POWERED_INTERFACE_OK);}
#endif
