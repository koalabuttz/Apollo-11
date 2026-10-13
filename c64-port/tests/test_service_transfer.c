/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "service_transfer_check.h"
#include "service_transfer_cases.h"
#include "service_transfer_vectors.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<svt_cases();++n)failures+=svt_check(n,svt_digests[n],&checks);
 printf("service-transfer: %u cases; %u checks, %lu failures\n",(unsigned)svt_cases(),(unsigned)checks,failures);
 return failures!=0;}
