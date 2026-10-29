/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_navframe_cases.h"
#include "sband_navframe_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;
 sband_navframe_input(&state,&io,id,&mission);state.words[at]=value;before=state;saved=io;
 return !sband_navframe(&state,&io,mission)&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);
}
int main(void){unsigned n;p66_bias_io io,old;enum apollo_fixed_mission mission;int status=sband_navframe_target_check(&n);
 if(status){fprintf(stderr,"frozen bodyframe failure %d row %u\n",status,n);return 1;}
 if(!reject(0,5,0)||!reject(65536,0164,0)||!reject(0,3,1)||!reject(65536,6,1)||!reject(0,7,1)||!reject(0,01000,0100000)||!reject(65536,067,1)||!reject(0,0120,0402)||!reject(0,023,1)||!reject(0,0166,0402)||!reject(65536,0165,0)||!reject(0,0163,0))return 2;
 sband_navframe_input(&state,&io,0,&mission);io.superbank=0;before=state;old=io;
 if(sband_navframe(&state,&io,mission)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 3;
 sband_navframe_input(&state,&io,0,&mission);io.interrupt_enabled=2;before=state;old=io;
 if(sband_navframe(&state,&io,mission)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 4;
 if(sband_navframe(0,&io,mission)||sband_navframe(&state,0,mission)||sband_navframe(&state,&io,(enum apollo_fixed_mission)2)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 5;
 puts("PASS original antenna body-axis/UNIT caller full-image fixtures and unchanged unsupported images");return 0;
}
