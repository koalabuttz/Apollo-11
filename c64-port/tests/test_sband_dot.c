/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_dot_cases.h"
#include "sband_dot_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){agc_word irq,super,fixed[6],saved[6];uint8_t use_fixed;enum apollo_fixed_mission mission;
 sband_dot_input(&state,&irq,&super,fixed,&use_fixed,id,&mission);state.words[at]=value;before=state;memcpy(saved,fixed,sizeof fixed);
 return !sband_dot(&state,mission,use_fixed?fixed:0)&&!memcmp(&state,&before,sizeof state)&&!memcmp(fixed,saved,sizeof fixed);
}
int main(void){unsigned n,g;int status=sband_dot_target_check(&n);agc_word irq,super,fixed[6];uint8_t use_fixed;enum apollo_fixed_mission mission;
 if(status){fprintf(stderr,"frozen DOT failure %d row %u\n",status,n);return 1;}
 for(g=0;g<4u;++g){uint32_t id=g*65536u;if(!reject(id,5,0)||!reject(id,0163,077777)||!reject(id,3,1)||!reject(id,6,1)||!reject(id,7,1)||!reject(id,01000,0100000)||!reject(id,0116,0))return 2;}
 sband_dot_input(&state,&irq,&super,fixed,&use_fixed,65536,&mission);before=state;fixed[5]=0100000;
 if(sband_dot(&state,mission,fixed)||memcmp(&state,&before,sizeof state))return 3;
 sband_dot_input(&state,&irq,&super,fixed,&use_fixed,65536,&mission);state.words[0116]=03773;before=state;
 if(sband_dot(&state,mission,fixed)||memcmp(&state,&before,sizeof state))return 4;
 sband_dot_input(&state,&irq,&super,fixed,&use_fixed,0,&mission);state.words[0116]=01401;state.words[3]=state.words[6]=0;before=state;
 if(sband_dot(&state,mission,0)||memcmp(&state,&before,sizeof state))return 5;
 if(sband_dot(0,mission,0)||sband_dot(&state,(enum apollo_fixed_mission)2,0)||memcmp(&state,&before,sizeof state))return 6;
 puts("PASS exact original canonical vector DOT and unchanged unsupported images");return 0;
}
