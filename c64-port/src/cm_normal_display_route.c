/* SPDX-License-Identifier: MIT */
#include "cm_normal_display.h"
#include "cm_nv85.h"
#include "cm_minutes_seconds.h"
#include "cm_clockjob.h"
#include "cm_display_control.h"
#include "cm_makeplay.h"
#include "cm_basic_priority.h"
#include "cm_basic_wake.h"
#include "cm_basic_finish.h"
#include "cm_scale.h"
uint8_t cm_normal_display_step(apollo_overlay_state *s,cm_native_io *io,uint8_t p){exec_address address;exec_transfer transfer;enum exec_status status;if(s==0||io==0)return 0;if(s->words[0371]==01450u&&p>=21u){if(p<=26u)return cm_minutes_seconds_phase(s,io,(uint8_t)(p-21u));if(p==27u)return cm_nv85_next(s,io);if(p==28u)return cm_normal_display_caller(s,io,5);if(p==29u){status=cm_basic_finish(s,io,&transfer);return(uint8_t)(status==EXEC_IDLE);}return 0;}switch(p){case 0:return(uint8_t)(cm_clockjob_select(s,io)==CM_CLOCKJOB_NORMAL);case 1:return(uint8_t)(cm_display_control_enter(s,io)==CM_DISPLAY_CONTROL_OK);case 2:return(uint8_t)(cm_makeplay_priority(s,io)==CM_MAKEPLAY_PRIORITY);case 3:return(uint8_t)(cm_basic_priority_change(s,io,&address)==EXEC_OK);case 4:return(uint8_t)(cm_makeplay_select(s,io)==CM_MAKEPLAY_WAKE);case 5:status=cm_basic_wake(s,io);return(uint8_t)(status==EXEC_OK||status==EXEC_NOT_FOUND);case 6:return(uint8_t)(cm_makeplay_wake_return(s,io)==CM_MAKEPLAY_PLAY);case 7:return cm_normal_display_caller(s,io,0);case 8:return cm_normal_display_caller(s,io,1);case 9:return cm_nv85_verb(s,io);case 10:return cm_normal_display_caller(s,io,2);case 11:return cm_normal_display_caller(s,io,3);case 12:case 16:case 20:return cm_normal_display_caller(s,io,4);case 13:case 17:case 21:return(uint8_t)(cm_scale_output(s,io,DSKY_DP3OUT)==EXEC_OK);case 14:case 18:case 22:return cm_nv85_decimal(s,io);case 15:case 19:case 23:return cm_nv85_next(s,io);case 24:return cm_normal_display_caller(s,io,5);case 25:status=cm_basic_finish(s,io,&transfer);return(uint8_t)(status==EXEC_IDLE);default:return 0;}}
