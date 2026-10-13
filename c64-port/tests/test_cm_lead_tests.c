/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_lead_tests_check.h"
#include "cm_lead_tests_cases.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<CLT_CASES;++n)failures+=clt_check(n,&checks);
 printf("cm-lead-tests: %u cases; %u checks, %lu failures\n",(unsigned)CLT_CASES,(unsigned)checks,failures);
 return failures!=0;}
