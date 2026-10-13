/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_entries_check.h"
#include "waitlist_entries_cases.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<wle_cases();++n)failures+=wle_check(n,&checks);
 printf("waitlist-entries: %u cases; %u checks, %lu failures\n",(unsigned)wle_cases(),(unsigned)checks,failures);
 return failures!=0;}
