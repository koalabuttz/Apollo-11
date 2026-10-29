/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "ephemeris_time_scale_target.h"
int main(void){unsigned completed;int status=ephemeris_time_scale_target_check(&completed);if(status){fprintf(stderr,"scalar SR14 original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original scalar SR14 states and13 immutable image guards plus null\n",completed);return 0;}
