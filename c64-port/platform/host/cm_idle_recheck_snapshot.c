/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_idle_recheck_cases.h"
#include "../../tools/cm_priority_full_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_IDLE_RECHECK_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_transfer next;cm_idle_recheck_input(&s,&io,n);if(cm_idle_recheck(&s,&io,&next)!=EXEC_OK)return 2;cm_priority_full_dump(n,&s,&io,&next);}return 0;}
