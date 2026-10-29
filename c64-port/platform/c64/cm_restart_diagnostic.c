/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_restart_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
static cm_restart_case state;
int main(void) { unsigned errors;MAIL[0]=0;puts("COMANCHE RESTART FOUNDATION");errors=cm_rs_check(&state);MAIL[1]=(uint8_t)errors;MAIL[2]=(uint8_t)(errors>>8);MAIL[3]=0;MAIL[4]=5;puts(errors?"CM RESTART FAILED":"CM 1280 STATES PASS");MAIL[0]=0xa5;return errors; }
