/* SPDX-License-Identifier: MIT */
#include "cm_startup_check.h"
#include <stdio.h>
#define MAIL ((volatile uint8_t *)0xc040)
static apollo_overlay_state image;
static cm_native_io io;
static cm_startup_ports ports;
int main(void) {
 unsigned errors;MAIL[0]=0;puts("CM SLAP1 / DOFSTART / CLEANUP");errors=cm_startup_check(&image,&io,&ports);
 MAIL[1]=(uint8_t)errors;MAIL[2]=(uint8_t)(errors>>8);MAIL[3]=128;MAIL[4]=2;
 puts(errors?"CM STARTUP FAILED":"CM640WHOLEIMAGE PASS");MAIL[0]=0xa5;return errors;
}
