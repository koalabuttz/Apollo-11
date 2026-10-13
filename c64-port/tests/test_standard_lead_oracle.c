/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "standard_lead_oracle_check.h"
#include "standard_lead_oracle_cases.h"
#include "standard_lead_oracle_vectors.h"
int main(void){uint16_t n,checks=0;unsigned long failures=0;
 for(n=0;n<SLO_CASES;++n)failures+=slo_check(n,slo_digests[n],&checks);
 printf("standard-lead-oracle: %u cases; %u checks, %lu failures\n",(unsigned)SLO_CASES,(unsigned)checks,failures);
 return failures!=0;}
