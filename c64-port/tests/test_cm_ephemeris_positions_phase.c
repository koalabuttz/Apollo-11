/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_ephemeris_positions_phase_target.h"
int main(void){unsigned n;int status=cm_ephemeris_positions_phase_target_check(&n);if(status){fprintf(stderr,"Solar phase checkpoint failed %u status%d\n",n,status);return 1;}printf("PASS%u source-only complete Solar phase checkpoints and immutable wrong-PC/null guards\n",n);return 0;}
