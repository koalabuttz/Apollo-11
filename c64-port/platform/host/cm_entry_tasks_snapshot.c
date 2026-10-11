/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_entry_tasks_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_ENTRYTASK_CASES;++n){enum cm_wl_status status;cm_entrytask_input(&s,&io,(uint16_t)n);status=cm_setjtag_image(&s,&io);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
