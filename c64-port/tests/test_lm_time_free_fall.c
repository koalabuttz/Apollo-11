/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_time_free_fall_target.h"
#include "lm_time_free_fall_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned completed,steps;int status=lm_time_free_fall_target_check(&state,&expected,&completed);
 if(status){fprintf(stderr,"lm-time-free-fall: original-chain check failed after %u chains, status %d\n",completed,status);return 1;}
 status=lm_time_free_fall_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"lm-time-free-fall: phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("PASS %u frozen original TIME_OF_FREE_FALL call chains, %u admission guards and %u frozen phase steps\n",completed,LM_TIME_FREE_FALL_GUARDS,steps);return 0;}
