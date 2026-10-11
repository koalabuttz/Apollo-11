/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_direct_launch_check.h"
static apollo_overlay_state image,expected;
int main(void){uint8_t n;uint16_t errors=0;for(n=0;n<24;++n)errors=(uint16_t)(errors+p66_direct_launch_check(n,&image,&expected));printf("P66 DIRECT:24 complete original images, real adoption, pending NEWJOB/IRQ:%u failures\n",errors);return errors!=0;}
