/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_trig_cases.h"
#include "sband_trig_target.h"
static apollo_overlay_state m,before;
static int reject(unsigned at,agc_word value){agc_word irq,super;sband_trig_input(&m,&irq,&super,0,APOLLO_FIXED_LM);m.words[at]=value;before=m;return !sband_trig(&m,APOLLO_FIXED_LM,0)&&!memcmp(&m,&before,sizeof m);}
int main(void){unsigned n;int status=sband_trig_target_check(&n);if(status){fprintf(stderr,"frozen trig failure%d row%u\n",status,n);return 1;}if(!reject(5,0)||!reject(3,1)||!reject(4,02000)||!reject(6,1)||!reject(7,1)||!reject(0163,077776)||!reject(0163,077777)||!reject(01000,0100000)||!reject(2,0100000))return 2;puts("PASS canonical original trig full-image fixtures and unchanged unsupported images");return 0;}
