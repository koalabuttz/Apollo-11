/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_scale_check.h"
static apollo_overlay_state s,before;
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_SCALE_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_scale_check(n,&s,&before));printf("CM fresh decimal output scaling: 64 complete2050-field original states, %u failures\n",failures);return failures!=0;}
