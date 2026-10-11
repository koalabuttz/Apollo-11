/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_powered_interface_cases.h"
#include "cm_powered_interface.h"
#include "cm_powered_trig.h"
#include "cm_powered_axis.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint8_t selector,uint8_t phase){uint16_t h[4]={n,selector,phase,irq};return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(int argc,char **argv){uint8_t selector;uint32_t limit=CM_POWERED_INTERFACE_CASES;(void)argv;if(argc>1)limit=1;for(selector=0;selector<8u;++selector){uint32_t n;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint8_t phase=0,step;cm_powered_interface_input(&m,&irq,(uint16_t)n,selector);if(cm_powered_interface_start(&m,selector)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;if(selector<6u){if(cm_powered_cdu_start(&m,(uint8_t)((selector&1u)==0))!=CM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;for(step=1;step<10u;++step){enum cm_powered_trig_status result=step%3u==1u?cm_powered_cdu_cosine(&m):step%3u==2u?cm_powered_cdu_sine(&m):cm_powered_cdu_next(&m);if(result!=CM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;}if(selector>=2u&&(cm_powered_interface_axis(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,selector,phase++)))return 2;}
if(selector>=2u){if(cm_powered_axis(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,(uint16_t)n,selector,phase++)||cm_powered_axis_return(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;}
if(cm_powered_interface_reentry(&m,&irq)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;
if(selector>=2u&&(cm_powered_interface_load(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,selector,phase++)))return 2;
if(cm_powered_interface_return(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,selector,phase++))return 2;}}return 0;}
