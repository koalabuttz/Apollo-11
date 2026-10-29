/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_linear_cases.h"
#include "sband_linear_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){agc_word irq,super,fixed[6],saved[6];uint8_t use_fixed;enum apollo_fixed_mission mission;enum sband_vector_operation op;
 sband_linear_input(&state,&irq,&super,fixed,&use_fixed,id,&mission,&op);state.words[at]=value;before=state;memcpy(saved,fixed,sizeof fixed);
 return !sband_vector_linear(&state,mission,op,use_fixed?fixed:0)&&!memcmp(&state,&before,sizeof state)&&!memcmp(fixed,saved,sizeof fixed);
}
int main(void){unsigned n,g;int status=sband_linear_target_check(&n);agc_word irq,super,fixed[6];uint8_t use_fixed;enum apollo_fixed_mission mission;enum sband_vector_operation op;
 if(status){fprintf(stderr,"frozen linear failure %d row %u\n",status,n);return 1;}
 for(g=0;g<16u;++g){uint32_t id=g*8192u;if(!reject(id,5,0)||!reject(id,0163,077777)||!reject(id,3,1)||!reject(id,6,1)||!reject(id,7,1)||!reject(id,01000,0100000)||!reject(id,0116,0))return 2;}
 sband_linear_input(&state,&irq,&super,fixed,&use_fixed,32768,&mission,&op);before=state;fixed[5]=0100000;
 if(sband_vector_linear(&state,mission,op,fixed)||memcmp(&state,&before,sizeof state))return 3;
 puts("PASS exact original canonical vector multiply/add/subtract and unchanged unsupported images");return 0;
}
