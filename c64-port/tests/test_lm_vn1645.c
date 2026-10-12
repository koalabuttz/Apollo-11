/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_vn1645_target.h"
#include "lm_vn1645_phase_target.h"
static apollo_overlay_state state,expected;
int main(void){unsigned completed,steps;int status=lm_vn1645_target_check(&state,&expected,&completed);
 if(status){fprintf(stderr,"lm-vn1645: original-entry check failed after %u entries, status %d\n",completed,status);return 1;}
 status=lm_vn1645_phase_check(&state,&expected,&steps);
 if(status){fprintf(stderr,"lm-vn1645: phase-step check failed after %u steps, status %d\n",steps,status);return 1;}
 printf("PASS %u frozen original VN1645 entries, %u admission guards and %u frozen phase steps\n",completed,LM_VN1645_GUARDS,steps);return 0;}
