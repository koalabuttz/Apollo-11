/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_launch_phased.h"
#include "p66_vertical_start.h"
#include "p66_launch_cases.h"
static apollo_overlay_state state;
static void dump(unsigned n,unsigned stage,unsigned status,p66_bias_io *io){unsigned i;printf("%u %u %u",n,stage,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",state.words[i]);printf(" %u %u\n",io->superbank,io->interrupt_enabled);}
int main(void){unsigned n;for(n=0;n<P66_LAUNCH_CASE_COUNT;++n){p66_bias_io io;enum p66_launch_status status;p66_launch_prepare(&state,(uint16_t)n);io.superbank=(uint16_t)((n/5u)%4u*020u);io.interrupt_enabled=(uint16_t)(n%2u);status=p66_launch_setup_phase(&state,&io);dump(n,0,status,&io);if(status!=P66_LAUNCH_OK)continue;status=p66_launch_bias_phase(&state,&io);dump(n,1,status,&io);if(status!=P66_LAUNCH_OK)return 2;status=p66_vertical_start(&state,&io)==WL_OK?P66_LAUNCH_OK:P66_LAUNCH_WAIT_FULL;dump(n,2,status,&io);}return 0;}
