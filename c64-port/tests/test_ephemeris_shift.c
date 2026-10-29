/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "ephemeris_shift_target.h"
int main(void){unsigned completed;int status=ephemeris_shift_target_check(&completed);if(status){fprintf(stderr,"scalar SL5 SL8 SL16 original-state check failed at%u status%d\n",completed,status);return 1;}printf("PASS%u complete original scalar SL5 SL8 SL16 states and13 immutable image guards plus null\n",completed);return 0;}
