/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_basic_check.h"
int main(void){uint16_t errors=p66_basic_check();printf("P66 BASIC: 64 full original display checkpoints, real ENDJOB/relay and 100cs RODTASK admission: %u failures\n",errors);return errors!=0;}
