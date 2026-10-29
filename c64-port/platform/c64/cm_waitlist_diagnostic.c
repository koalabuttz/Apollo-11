/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_waitlist_check.h"
#define MAIL ((volatile unsigned char *)0xc040)
static cm_waitlist state;
int main(void) {
 unsigned failures;
 MAIL[0]=0; puts("COMANCHE WAITLIST / TASKS");
 failures=cm_waitlist_check(&state,0);
 MAIL[1]=(unsigned char)failures; MAIL[2]=(unsigned char)(failures>>8);
 MAIL[3]=(unsigned char)CM_WAIT_CHECKPOINT_COUNT; MAIL[4]=(unsigned char)(CM_WAIT_CHECKPOINT_COUNT>>8);
 puts(failures?"CM WAITLIST FAILED":"CM 306 FULL STATES PASS");
 MAIL[0]=0xa5; return failures;
}
