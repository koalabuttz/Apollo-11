/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_length_cases.h"
#include "sband_length_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;
 sband_length_input(&state,&io,id,&mission);state.words[at]=value;before=state;saved=io;
 return !sband_length(&state,&io,mission)&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);
}
int main(void){unsigned n;apollo_overlay_state saved;p66_bias_io io,old;enum apollo_fixed_mission mission;int status=sband_length_target_check(&n);
 if(status){fprintf(stderr,"frozen ABVAL failure %d row %u\n",status,n);return 1;}
 if(!reject(0,5,0)||!reject(65536,0163,0)||!reject(0,3,1)||!reject(65536,6,1)||!reject(0,7,1)||!reject(0,01000,0100000)||!reject(65536,4,02000)||!reject(0,0120,0402)||!reject(0,2,0100000))return 2;
 sband_length_input(&state,&io,0,&mission);io.interrupt_enabled=2;before=state;old=io;
 if(sband_length(&state,&io,mission)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 3;
 sband_length_input(&state,&io,0,&mission);io.superbank=0200;before=state;old=io;
 if(sband_length(&state,&io,mission)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 4;
 saved=state;
 if(sband_length(0,&io,mission)||sband_length(&state,0,mission)||sband_length(&state,&io,(enum apollo_fixed_mission)2)||memcmp(&state,&saved,sizeof state)||memcmp(&io,&old,sizeof io))return 5;
 puts("PASS original canonical vector ABVAL full-image fixtures and unchanged unsupported images");return 0;
}
