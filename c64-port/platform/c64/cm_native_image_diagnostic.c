/* SPDX-License-Identifier: MIT */
#include "cm_native_image_check.h"
#include <stdio.h>
#define MAIL ((volatile uint8_t *)0xc040)
static apollo_overlay_state image;
static cm_native_io io;
static cm_rn_run run;
int main(void) {
 unsigned errors;MAIL[0]=0;puts("CM SOLE FLIGHT WORD IMAGE");errors=cm_image_check(&image,&io,&run);
 MAIL[1]=(uint8_t)errors;MAIL[2]=(uint8_t)(errors>>8);MAIL[3]=64;MAIL[4]=1;
 puts(errors?"CM IMAGE FAILED":"CM320BOUNDARIES PASS");MAIL[0]=0xa5;return errors;
}
