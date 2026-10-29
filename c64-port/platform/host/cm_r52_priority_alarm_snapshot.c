/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_r52_priority_alarm_cases.h"
#include "cm_r52_priority_alarm_dump.h"
int main(void){uint32_t n;unsigned phase;for(n=0;n<CM_R52_PRIORITY_ALARM_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_transfer next;cm_r52_priority_alarm_input(&s,&io,n);for(phase=0;phase<7u;++phase){enum exec_status status=cm_r52_priority_alarm_step(&s,&io,phase,&next);if(status==EXEC_OUT_OF_SCOPE){fprintf(stderr,"r52 scope%lu phase%u PC%o\n",(unsigned long)n,phase,s.words[5]);return 2;}cm_r52_priority_alarm_dump(n,phase,status,&s,&io);if(status!=EXEC_OK)break;}}return 0;}
