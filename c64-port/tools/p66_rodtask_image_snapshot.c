/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_rodtask_image.h"
#include "p66_basic_task_cases.h"
int main(void){uint16_t n;for(n=0;n<P66_BASIC_TASK_CASE_COUNT;++n){apollo_overlay_state image;p66_bias_io io={(uint16_t)(((n/432u)%4u)*020u),(uint16_t)(n%2u)};enum exec_status status;unsigned i;p66_basic_task_prepare(&image,n);status=p66_rodtask_image(&image,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
