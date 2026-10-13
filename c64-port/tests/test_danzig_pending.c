/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "danzig_pending_check.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<dzp_row_count();++n)failures+=dzp_check(n,&checks);
 printf("danzig-pending: %u RTB-left pairs; %u checks, %lu failures\n",(unsigned)dzp_row_count(),(unsigned)checks,failures);
 return failures!=0;}
