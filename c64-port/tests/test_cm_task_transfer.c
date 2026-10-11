/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_task_transfer_check.h"
int main(void){unsigned n=cm_task_transfer_check();printf("1024 original CM task transfer full-image states: %s\n",n?"FAIL":"PASS");return n?1:0;}
