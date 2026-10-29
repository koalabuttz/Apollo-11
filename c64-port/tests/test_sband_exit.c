/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_exit_cases.h"
#include "sband_exit_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;sband_exit_input(&state,&io,id,&mission);state.words[at]=value;before=state;saved=io;return !sband_exit(&state,&io,mission)&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);}
int main(void){unsigned n;p66_bias_io io,old;enum apollo_fixed_mission mission;int status=sband_exit_target_check(&n);if(status){fprintf(stderr,"EXIT fixture failure %d row %u\n",status,n);return 1;}
 if(!reject(0,5,0)||!reject(65536,0164,0)||!reject(0,3,1)||!reject(65536,6,1)||!reject(0,7,1)||!reject(0,01000,0100000)||!reject(65536,067,1)||!reject(0,023,1)||!reject(65536,0165,0)||!reject(0,0163,1))return 2;
 sband_exit_input(&state,&io,0,&mission);before=state;old=io;io.superbank=0;old=io;if(sband_exit(&state,&io,mission)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 3;
 if(sband_exit(0,&io,mission)||sband_exit(&state,0,mission)||sband_exit(&state,&io,(enum apollo_fixed_mission)2))return 4;
 puts("PASS both original antenna EXIT full-image fixtures and unchanged unsupported states");return 0;}
