/* SPDX-License-Identifier: MIT */
#include "cm_restart_native_check.h"
#include <stdio.h>
#define MAIL ((volatile uint8_t *)0xc040)
static cm_rn_run run;
int main(void) {
 unsigned errors;MAIL[0]=0;puts("CM RESTART SELECTED C CALLBACKS");errors=cm_rn_check(&run);
 MAIL[1]=(uint8_t)errors;MAIL[2]=(uint8_t)(errors>>8);MAIL[3]=64;MAIL[4]=1;
 puts(errors?"CM RESTART CALLBACK FAILED":"CM320CHECKPOINTS PASS");MAIL[0]=0xa5;return errors;
}
