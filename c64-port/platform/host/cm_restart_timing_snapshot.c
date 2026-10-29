/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_restart_timing_cases.h"
static apollo_overlay_state state;
int main(void){unsigned n,i;
 for(n=0;n<CM_RSTIME_CASES;++n){enum cm_restart_time_exit status;cm_rstime_input(&state,(uint16_t)n);
 status=n<2048u?cm_restart_wait_delay(&state,(uint8_t)(n%6u)):cm_restart_long_delay(&state);
 printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",state.words[i]);putchar('\n');
 }return 0;
}
