/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_flow_cases.h"
#include "sband_flow_target.h"
static apollo_overlay_state m,before;
static int reject(unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;enum sband_flow_entry entry;sband_flow_input(&m,&io,0,&mission,&entry);m.words[at]=value;before=m;saved=io;return !sband_flow(&m,&io,mission,entry)&&!memcmp(&m,&before,sizeof m)&&!memcmp(&io,&saved,sizeof io);}
static int pipeline_reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;enum sband_flow_entry entry;sband_flow_input(&m,&io,id,&mission,&entry);m.words[at]=value;before=m;saved=io;return !sband_flow(&m,&io,mission,entry)&&!memcmp(&m,&before,sizeof m)&&!memcmp(&io,&saved,sizeof io);}
int main(void){unsigned n;int status=sband_flow_target_check(&n);if(status){fprintf(stderr,"frozen flow failure%d row%u\n",status,n);return 1;}
 if(!reject(5,0)||!reject(3,1)||!reject(4,0)||!reject(6,0)||!reject(7,1)||!reject(01000,0100000)||!reject(067,1)||!reject(0120,0402))return 2;
 if(!pipeline_reject(8192,0165,0)||!pipeline_reject(36864,023,1)||!pipeline_reject(36864,067,1)||!pipeline_reject(36864,0120,0))return 3;
 puts("PASS original flow full-image fixtures and unchanged unsupported stages/pipelines");return 0;}
