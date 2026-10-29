/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_nbsm_cases.h"
#include "cm_nbsm.h"
#include "cm_powered_interface.h"
#include "cm_powered_trig.h"
#include "cm_powered_axis.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint16_t phase){uint16_t h[3]={n,phase,irq};return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(int argc,char **argv){uint32_t n,limit=CM_NBSM_CASES;(void)argv;if(argc>1)limit=1;for(n=0;n<limit;++n){apollo_overlay_state m;agc_word irq;uint8_t phase=0,step;cm_nbsm_input(&m,&irq,(uint16_t)n);for(step=0;step<5u;++step)if(cm_nbsm(&m,(enum cm_nbsm_phase)step)!=CM_NBSM_OK||row(&m,irq,(uint16_t)n,phase++))return 2;
if(cm_powered_interface_start(&m,5)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_cdu_start(&m,0)!=CM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,phase++))return 2;
for(step=1;step<10u;++step){enum cm_powered_trig_status result=step%3u==1u?cm_powered_cdu_cosine(&m):step%3u==2u?cm_powered_cdu_sine(&m):cm_powered_cdu_next(&m);if(result!=CM_POWERED_TRIG_OK||row(&m,irq,(uint16_t)n,phase++))return 2;}
if(cm_powered_interface_axis(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_axis(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_axis_return(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_interface_reentry(&m,&irq)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_interface_load(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,phase++)||cm_powered_interface_return(&m)!=CM_POWERED_INTERFACE_OK||row(&m,irq,(uint16_t)n,phase++)||cm_nbsm(&m,CM_NBSM_STORE_RETURN)!=CM_NBSM_OK||row(&m,irq,(uint16_t)n,phase++))return 2;}
return 0;}
