/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_zero_check.h"
int main(void) { uint16_t n,failures=0; waitlist w; for(n=0;n<WL_ZERO_CASE_COUNT;++n) failures=(uint16_t)(failures+wl_zero_check(n,&w)); printf("WAITLIST ZERO: 256 FULL STATES, %u FAILURES\n",failures); return failures!=0; }
