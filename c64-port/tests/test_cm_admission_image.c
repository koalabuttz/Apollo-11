/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_admission_image_check.h"
int main(void){unsigned n=cm_admission_image_check();printf("1024 original CM admission full-image states: %s\n",n?"FAIL":"PASS");return n?1:0;}
