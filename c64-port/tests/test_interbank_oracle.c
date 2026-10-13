/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "interbank_oracle_check.h"
#include "interbank_oracle_cases.h"
#include "interbank_oracle_vectors.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<ibo_cases();++n)failures+=ibo_check(n,ibo_digests[n],&checks);
 printf("interbank-oracle: %u cases; %u checks, %lu failures\n",(unsigned)ibo_cases(),(unsigned)checks,failures);
 return failures!=0;}
