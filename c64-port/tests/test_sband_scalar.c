/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_scalar_cases.h"
#include "sband_scalar_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){agc_word irq,super,fixed[2],saved[2];uint8_t use_fixed;enum apollo_fixed_mission mission;enum sband_scalar_operation op;
 sband_scalar_input(&state,&irq,&super,fixed,&use_fixed,id,&mission,&op);state.words[at]=value;before=state;memcpy(saved,fixed,sizeof fixed);
 return !sband_scalar(&state,mission,op,use_fixed?fixed:0)&&!memcmp(&state,&before,sizeof state)&&!memcmp(fixed,saved,sizeof fixed);
}
int main(void){unsigned n,g;int status=sband_scalar_target_check(&n);agc_word irq,super,fixed[2];uint8_t use_fixed;enum apollo_fixed_mission mission;enum sband_scalar_operation op;
 if(status){fprintf(stderr,"frozen DOT failure %d row %u\n",status,n);return 1;}
 for(g=0;g<12u;++g){uint32_t id=g*65536u;if(!reject(id,5,0)||!reject(id,0163,077777)||!reject(id,3,1)||!reject(id,6,1)||!reject(id,7,1)||!reject(id,01000,0100000)||!reject(id,0116,0))return 2;}
 sband_scalar_input(&state,&irq,&super,fixed,&use_fixed,65536,&mission,&op);before=state;fixed[1]=0100000;
 if(sband_scalar(&state,mission,op,fixed)||memcmp(&state,&before,sizeof state))return 3;
 sband_scalar_input(&state,&irq,&super,fixed,&use_fixed,65536,&mission,&op);state.words[0116]=03777;before=state;
 if(sband_scalar(&state,mission,op,fixed)||memcmp(&state,&before,sizeof state))return 4;
 sband_scalar_input(&state,&irq,&super,fixed,&use_fixed,0,&mission,&op);state.words[0116]=01401;state.words[3]=state.words[6]=0;before=state;
 if(sband_scalar(&state,mission,op,0)||memcmp(&state,&before,sizeof state))return 5;
 if(sband_scalar(0,mission,op,0)||sband_scalar(&state,(enum apollo_fixed_mission)2,op,0)||memcmp(&state,&before,sizeof state))return 6;
 puts("PASS exact original canonical DP DMP/DAD/DSU and unchanged unsupported images");return 0;
}
