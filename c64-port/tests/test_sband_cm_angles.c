/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_cm_angles_cases.h"
#include "sband_cm_angles_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;
 sband_cm_angles_input(&state,&io,id,&mission);state.words[at]=value;before=state;saved=io;
 return !(id>=65536u?sband_cm_angles_null(&state,&io):sband_cm_angles(&state,&io))&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);
}
int main(void){unsigned n;p66_bias_io io,old;enum apollo_fixed_mission mission;int status=sband_cm_angles_target_check(&n);
 if(status){fprintf(stderr,"frozen plane failure %d row %u\n",status,n);return 1;}
 if(!reject(0,5,0)||!reject(65536,0164,0)||!reject(0,3,1)||!reject(65536,6,1)||!reject(0,7,1)||!reject(0,01000,0100000)||!reject(65536,067,1)||!reject(0,0120,0402)||!reject(0,023,1)||!reject(0,0166,0402)||!reject(65536,0165,0)||!reject(0,0163,0))return 2;
 sband_cm_angles_input(&state,&io,0,&mission);io.superbank=0;before=state;old=io;
 if(sband_cm_angles(&state,&io)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 3;
 sband_cm_angles_input(&state,&io,0,&mission);io.interrupt_enabled=2;before=state;old=io;
 if(sband_cm_angles(&state,&io)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 4;
 if(sband_cm_angles(0,&io)||sband_cm_angles(&state,0)||memcmp(&state,&before,sizeof state)||memcmp(&io,&old,sizeof io))return 5;
 puts("PASS original antenna CM yaw/pitch caller full-image fixtures and unchanged unsupported images");return 0;
}
