/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_ephemeris_target.h"
#include "lm_ephemeris_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned completed,steps;int status=lm_ephemeris_target_check(&state,&expected,&completed);
 if(status){fprintf(stderr,"lm-ephemeris: original-state check failed after %u states, status %d\n",completed,status);return 1;}
 status=lm_ephemeris_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"lm-ephemeris: phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("PASS %u frozen original LSPOS/LUNVEL states, %u admission guards and %u frozen phase steps\n",completed,LM_EPHEMERIS_GUARDS,steps);return 0;}
