/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_conic_timethet_target.h"
#include "lm_conic_timethet_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned completed,steps;int status=lm_conic_timethet_target_check(&state,&expected,&completed);
 if(status){fprintf(stderr,"lm-conic-timethet: original-entry check failed after %u entries, status %d\n",completed,status);return 1;}
 status=lm_conic_timethet_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"lm-conic-timethet: phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("PASS %u frozen original TIMETHET entries, %u admission guards and %u frozen phase steps\n",completed,LM_CONIC_TIMETHET_GUARDS,steps);return 0;}
