/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_keyboard_runtime_check.h"
static apollo_overlay_state s,w;
int main(void){uint16_t errors=0;uint8_t n;for(n=0;n<8;++n)errors=(uint16_t)(errors+cm_keyboard_runtime_check(n,&s,&w));printf("CM keyboard8 genuine lifecycles32 full source states:%u failures\n",errors);return errors!=0;}
