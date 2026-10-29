/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_moon_cases.h"
#include "sband_moon_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;
 sband_moon_input(&state,&io,id,&mission);state.words[at]=value;before=state;saved=io;
 return !sband_moon(&state,&io,mission)&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);
}
int main(void){unsigned n;int status=sband_moon_target_check(&n);
 if(status){fprintf(stderr,"frozen orbit failure %d row %u\n",status,n);return 1;}
 if(!reject(0,5,0)||!reject(65536,0164,0)||!reject(0,3,1)||!reject(65536,6,0)||!reject(0,7,1)||!reject(0,01000,0100000)||!reject(65536,067,1)||!reject(0,0120,0402)||!reject(0,023,1))return 2;
 puts("PASS original Moon-return vector caller full-image fixtures and unchanged unsupported images");return 0;
}
