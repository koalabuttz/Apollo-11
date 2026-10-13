/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "interbank_q_check.h"
#include "interbank_q_cases.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<IBQ_CASES;++n)failures+=ibq_check(n,&checks);
 printf("interbank-q: %u cases; %u checks, %lu failures\n",(unsigned)IBQ_CASES,(unsigned)checks,failures);
 return failures!=0;}
