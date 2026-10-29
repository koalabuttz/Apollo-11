/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_irq_cases.h"
#define MAIL ((volatile unsigned char *)0xc040)
int main(void) {
 unsigned failures;
 MAIL[0]=0; puts("COMANCHE IRQ ADMISSION");
 failures=cm_irq_check_cases(0);
 MAIL[1]=(unsigned char)failures; MAIL[2]=(unsigned char)(failures>>8);
 MAIL[3]=(unsigned char)CM_IRQ_CASE_COUNT; MAIL[4]=(unsigned char)(CM_IRQ_CASE_COUNT>>8);
 puts(failures?"CM IRQ ADMISSION CHECKS FAILED":"CM IRQ ADMISSION CHECKS PASS");
 MAIL[0]=0xa5; return failures;
}
