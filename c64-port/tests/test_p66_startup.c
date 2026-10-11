/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_startup_check.h"
int main(void) { unsigned n; uint16_t failures=0; p66_startup_case s; for(n=0;n<P66_STARTUP_TARGET_COUNT;++n) failures=(uint16_t)(failures+p66_startup_check((uint8_t)n,&s)); printf("P66 STARTUP: 128 FULL STATES, %u FAILURES\n",failures); return failures!=0; }
