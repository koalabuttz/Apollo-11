/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_application.h"
#include "p66_direct_launch.h"
#include "p66_direct_launch_cases.h"
static apollo_overlay_state image;static p66_application_binding binding;
int main(void){uint16_t n,i;for(n=0;n<P66_DIRECT_LAUNCH_CASE_COUNT;++n){p66_bias_io io={(uint16_t)((n/5u)%4u*020u),(uint16_t)(n%2u)};uint16_t status;p66_direct_launch_prepare(&image,n);memset(&binding,0,sizeof binding);status=p66_application_basic(&binding,&image,&io,n<1024?P66_DIRECT_LAUNCH_SERVICE:P66_DIRECT_VERTICAL_SERVICE);printf("%u %u",n,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
