/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "ephemeris_day_target.h"
int main(void){unsigned completed;int status=ephemeris_day_target_check(&completed);if(status){fprintf(stderr,"CSTODAY divide original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original CSTODAY divide states and13 immutable image guards plus null\n",completed);return 0;}
