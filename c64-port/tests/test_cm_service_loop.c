/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_service_loop_check.h"
#include "cm_service_loop_cases.h"
static apollo_overlay_state s,before;static cm_native_io io,oldio;
static enum cm_service_status recursive(const cm_service_call *c,apollo_overlay_state *image,cm_native_io *device,void *user){cm_service_loop *r=(cm_service_loop *)user;(void)c;(void)image;(void)device;return cm_service_step(r)==CM_SERVICE_REENTER?CM_SERVICE_ADVANCED:CM_SERVICE_FAULT;}
int main(void){unsigned fail=cm_service_loop_check();cm_service_loop r={&s,&io,recursive,0,0,0};r.user=&r;cm_service_loop_input(&s,&io,0);before=s;oldio=io;
 if(cm_service_step(&r)!=CM_SERVICE_ADVANCED||r.in_callback||r.completed_groups!=1u||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io))++fail;
 s.words[5]=02612;s.words[6]=050006;s.words[3]=03000;s.words[4]=050000;before=s;oldio=io;r.invoke=cm_service_builtin_invoke;
 if(cm_service_step(&r)!=CM_SERVICE_UNMAPPED||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io))++fail;
 printf("384 continuous original CM ISR task cases/1856 full-image states and callback guards: %s\n",fail?"FAIL":"PASS");return fail?1:0;
}
