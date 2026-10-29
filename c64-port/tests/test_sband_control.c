/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_control_cases.h"
#include "sband_control_target.h"
static apollo_overlay_state m,before;
static int reject(unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;enum sband_control_entry entry;sband_control_input(&m,&io,0,&mission,&entry);m.words[at]=value;before=m;saved=io;return !sband_control(&m,&io,mission,entry)&&!memcmp(&m,&before,sizeof m)&&!memcmp(&io,&saved,sizeof io);}
int main(void){unsigned n;int status=sband_control_target_check(&n);if(status){fprintf(stderr,"frozen control failure%d row%u\n",status,n);return 1;}if(!reject(5,0)||!reject(3,1)||!reject(4,0)||!reject(6,0)||!reject(7,1)||!reject(01000,0100000))return 2;puts("PASS original control full-image fixtures and unchanged unsupported images");return 0;}
