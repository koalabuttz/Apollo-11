/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_enema_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
static cm_enema_case state;
int main(void) { unsigned errors;MAIL[0]=0;puts("COMANCHE ENEMA / TVC TASK");errors=cm_enema_check(&state);MAIL[1]=(uint8_t)errors;MAIL[2]=(uint8_t)(errors>>8);MAIL[3]=0;MAIL[4]=16;puts(errors?"CM ENEMA FAILED":"CM4096STATES PASS");MAIL[0]=0xa5;return errors; }
