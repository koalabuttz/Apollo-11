/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_ephemeris_time_target.h"
int main(void){unsigned completed;int status=cm_ephemeris_time_target_check(&completed);if(status){fprintf(stderr,"ephemerides time original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original ephemerides time states and18 immutable image guards plus two null guards\n",completed);return 0;}
