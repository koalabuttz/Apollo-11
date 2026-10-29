/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_dispatch_check.h"
#define MAIL ((volatile unsigned char *)0xc040)
int main(void) {
 unsigned failures;
 MAIL[0]=0; puts("COMANCHE CONTEXTS / LIFECYCLE");
 failures=cm_dispatch_run_checks();
 MAIL[1]=(unsigned char)failures; MAIL[2]=(unsigned char)(failures>>8);
 MAIL[3]=(unsigned char)CM_CHECK_COUNT; MAIL[4]=(unsigned char)(CM_CHECK_COUNT>>8);
 puts(failures?"CM CONTEXT CHECKS FAILED":"CM CONTEXT CHECKS PASS");
 MAIL[0]=0xa5; return failures;
}
