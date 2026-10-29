/* SPDX-License-Identifier: MIT */
#include "cm_dyndisp.h"
#include "cm_powered_interface.h"
#include "cm_powered_trig.h"
#include "cm_powered_axis.h"
#include "matrix_native.h"
#include "gimbal_stage.h"
static uint8_t powered(apollo_overlay_state *s,agc_word *irq,uint8_t selector){uint8_t step;if(cm_powered_interface_start(s,selector)!=CM_POWERED_INTERFACE_OK)return 0;if(selector==0u){if(cm_powered_cdu_start(s,1)!=CM_POWERED_TRIG_OK)return 0;for(step=1;step<10u;++step){enum cm_powered_trig_status result=step%3u==1u?cm_powered_cdu_cosine(s):step%3u==2u?cm_powered_cdu_sine(s):cm_powered_cdu_next(s);if(result!=CM_POWERED_TRIG_OK)return 0;}}else if(cm_powered_axis(s)!=CM_POWERED_AXIS_OK||cm_powered_axis_return(s)!=CM_POWERED_AXIS_OK)return 0;if(cm_powered_interface_reentry(s,irq)!=CM_POWERED_INTERFACE_OK)return 0;if(selector==6u&&cm_powered_interface_load(s)!=CM_POWERED_INTERFACE_OK)return 0;return(uint8_t)(cm_powered_interface_return(s)==CM_POWERED_INTERFACE_OK);}
uint8_t cm_dyndisp_step(apollo_overlay_state *s,cm_native_io *io,uint8_t route){static const uint8_t stage[27]={0,1,2,3,4,5,6,7,255,8,9,254,254,254,254,10,253,11,252,252,252,252,12,13,14,15,16};agc_word matrix[18];uint8_t i;if(s==0||io==0||route>=27u)return 0;switch(stage[route]){case 255:return powered(s,&io->interrupt_enabled,0);case 253:return powered(s,&io->interrupt_enabled,6);case 254:return matrix_native_phase(s,APOLLO_FIXED_CM,0,s->words+01735,(uint8_t)(route-11u));case 252:for(i=0;i<18u;++i)if(!gimbal_fixed_read(APOLLO_FIXED_CM,(uint16_t)(18946u+i),matrix+i))return 0;return matrix_native_phase(s,APOLLO_FIXED_CM,0,matrix,(uint8_t)(route-18u));default:return cm_dyndisp_stage(s,io,stage[route]);}}
