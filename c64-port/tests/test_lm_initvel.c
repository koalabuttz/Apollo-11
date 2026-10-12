/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_initvel_target.h"
#include "lm_initvel_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned completed,steps;int status=lm_initvel_target_check(&state,&expected,&completed);
 if(status){fprintf(stderr,"lm-initvel: original-entry check failed after %u entries, status %d\n",completed,status);return 1;}
 status=lm_initvel_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"lm-initvel: phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("PASS %u frozen original INITVEL entries, %u admission guards and %u frozen phase steps\n",completed,LM_INITVEL_GUARDS,steps);return 0;}
