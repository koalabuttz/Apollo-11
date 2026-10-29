/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_longcall_cases.h"
static apollo_overlay_state image;
int main(void){unsigned n,i;for(n=0;n<CM_LONGCALL_CASES;++n){exec_address task;enum cm_longcall_status status;cm_longcall_input(&image,&task,(uint16_t)n);status=cm_longcall_phase_run(&image,(enum cm_longcall_phase)(n/1024u),task);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);putchar('\n');}return 0;}
