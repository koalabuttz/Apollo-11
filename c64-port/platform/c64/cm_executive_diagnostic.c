/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_executive_cases.h"
#include "cm_executive_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
int main(void) {
 unsigned n,failures=0;
 MAIL[0]=0; puts("COMANCHE 055 EXECUTIVE");
 for(n=0;n<CM_EXEC_CASE_COUNT;++n) if(!cm_case_check(n)) ++failures;
 MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
 MAIL[3]=(uint8_t)CM_EXEC_CASE_COUNT; MAIL[4]=(uint8_t)(CM_EXEC_CASE_COUNT>>8);
 puts(failures?"CM CHECKS FAILED":"1536 CM CHECKS PASS"); MAIL[0]=0xa5;
 return failures;
}
