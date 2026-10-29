/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_shift_cases.h"
#include "sband_shift_target.h"
static apollo_overlay_state state,before;
static int reject(unsigned at,agc_word value){agc_word irq,super;enum apollo_fixed_mission mission;uint8_t count;
 sband_shift_input(&state,&irq,&super,0,&mission,&count);state.words[at]=value;before=state;
 return !sband_shift_left(&state,mission,count)&&!memcmp(&state,&before,sizeof state);
}
int main(void){unsigned n;agc_word irq,super;enum apollo_fixed_mission mission;uint8_t count;int status=sband_shift_target_check(&n);
 if(status){fprintf(stderr,"frozen shift failure %d row %u\n",status,n);return 1;}
 if(!reject(5,0)||!reject(0163,0)||!reject(3,1)||!reject(6,1)||!reject(7,1)||!reject(01000,0100000)||!reject(020,020007))return 2;
 sband_shift_input(&state,&irq,&super,0,&mission,&count);before=state;
 if(sband_shift_left(&state,mission,0)||memcmp(&state,&before,sizeof state)||sband_shift_left(&state,mission,9)||memcmp(&state,&before,sizeof state)||sband_shift_left(&state,(enum apollo_fixed_mission)2,count)||memcmp(&state,&before,sizeof state))return 3;
 puts("PASS exact original short vector-left fixtures and unchanged unsupported images");return 0;
}
