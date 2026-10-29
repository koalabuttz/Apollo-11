/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_display_check.h"
static uint8_t screen[1002],color[1002];
int main(void){uint16_t failures;screen[0]=screen[1001]=color[0]=color[1001]=0xa5;failures=dsky_display_check(screen+1,color+1);if(screen[0]!=0xa5 || screen[1001]!=0xa5 || color[0]!=0xa5 || color[1001]!=0xa5)++failures;printf("DSKY NATIVE SCREEN: SOURCE MODE66, RELAY WIRING/SIGNS/BLANK/FLASH/LAMPS, %u FAILURES\n",failures);return failures!=0;}
