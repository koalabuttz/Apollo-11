/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_direct_pending_check.h"
static apollo_overlay_state image,expected;
int main(void){uint16_t errors=p66_direct_pending_check(&image,&expected);printf("P66 pending higher-priority NEWJOB+IRQ: full original2050-field source image:%u failures\n",errors);return errors!=0;}
