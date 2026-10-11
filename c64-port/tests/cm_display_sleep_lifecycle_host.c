/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_sleep_lifecycle_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t failures=0;uint8_t n;for(n=0;n<64u;++n)failures=(uint16_t)(failures+cm_display_sleep_lifecycle_check(n,&s,&before));printf("64 complete original CM display sleep lifecycle checkpoints: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
