/* SPDX-License-Identifier: MIT */
/* Host replay of every frozen C64 TIME_OF_FREE_FALL phase row (all groups
 * and parts), for tools/lm_time_free_fall_c64_cover.py. */
#include <stdio.h>
#include "lm_time_free_fall_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned steps;int status=lm_time_free_fall_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("%u frozen phase steps\n",steps);return 0;}
