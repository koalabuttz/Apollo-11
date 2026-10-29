/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_task_transfer_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_TRANSFER_CASES;++n){enum cm_task_transfer_exit status;cm_task_transfer_input(&s,&io,(uint16_t)n);status=cm_task_transfer(&s,&io,n<512u?CM_TASK_DISPATCH:CM_TASK_FINISH);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
