/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "ephemeris_add_target.h"
int main(void){unsigned completed;int status=ephemeris_add_target_check(&completed);if(status){fprintf(stderr,"TAD original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original TAD states and13 immutable image guards plus null\n",completed);return 0;}
