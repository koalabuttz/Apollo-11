/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_waitlist_image_check.h"
int main(void){unsigned n=cm_waitlist_image_check();printf("1024 original CM WAITLIST full-image states: %s\n",n?"FAIL":"PASS");return n?1:0;}
