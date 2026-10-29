/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_vector_cases.h"
#include "sband_vector_target.h"
static apollo_overlay_state state,before;
static int reject(unsigned at,agc_word value){agc_word irq,super;sband_vector_input(&state,&irq,&super,0,APOLLO_FIXED_LM);state.words[at]=value;before=state;return !sband_vector_cross(&state,APOLLO_FIXED_LM)&&memcmp(&state,&before,sizeof state)==0;}
static int reject_fixed(void){agc_word irq,super,factor[6];sband_vector_input(&state,&irq,&super,65536,APOLLO_FIXED_LM);sband_vector_fixed_input(factor,65536);before=state;factor[0]=0100000;if(sband_vector_cross_fixed(&state,APOLLO_FIXED_LM,factor)||memcmp(&state,&before,sizeof state))return 0;factor[0]=020000;state.words[0116]=03773;before=state;if(sband_vector_cross_fixed(&state,APOLLO_FIXED_LM,factor)||memcmp(&state,&before,sizeof state))return 0;if(sband_vector_cross_fixed(&state,APOLLO_FIXED_LM,0)||memcmp(&state,&before,sizeof state))return 0;return 1;}
int main(void){unsigned done;int status=sband_vector_target_check(&done);if(status){fprintf(stderr,"frozen failure %d row%u\n",status,done);return 1;}if(!reject(5,0)||!reject(7,1)||!reject(0163,0)||!reject(0163,077777)||!reject(0116,0)||!reject(0116,01773)||!reject(3,1)||!reject(6,1)||!reject(01000,0100000)||!reject_fixed())return 2;puts("PASS full original C64 fixtures and unchanged unsupported images");return 0;}
