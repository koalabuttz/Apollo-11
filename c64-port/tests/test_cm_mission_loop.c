/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_mission_loop_check.h"
#include "cm_mission_loop_cases.h"
static apollo_overlay_state s,before;static cm_native_io io;
int main(void){unsigned failures=cm_mission_loop_check();cm_mission_loop r={&s,&io,cm_mission_builtin_invoke,0,{0,0},{{0,0},0},0,1,0};cm_mission_loop_input(&s,&io,0);before=s;r.in_callback=1;if(cm_mission_step(&r)!=CM_SERVICE_REENTER||memcmp(&s,&before,sizeof s))++failures;r.in_callback=0;io.interrupt_enabled=0;if(cm_mission_step(&r)!=CM_SERVICE_RESUME||memcmp(&s,&before,sizeof s)||!r.pending_t3)++failures;printf("384 original complete IRQ/ISR/RESUME/DANZIG continuous sequences: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
