/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_ephemeris_solar_target.h"
int main(void){unsigned completed;int status=cm_ephemeris_solar_target_check(&completed);if(status){fprintf(stderr,"LUNPOS polynomial and return original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original LUNPOS polynomial and return states and19 immutable image guards plus two null guards\n",completed);return 0;}
