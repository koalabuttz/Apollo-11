/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_priority_display_cases.h"
#include "cm_priority_display_dump.h"
int main(void){uint32_t n;unsigned phase;for(n=0;n<CM_PRIORITY_DISPLAY_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_priority_display_input(&s,&io,n);for(phase=0;phase<3u;++phase){enum exec_status status=cm_priority_display_step(&s,&io,phase);if(status==EXEC_OUT_OF_SCOPE){fprintf(stderr,"prioritydisplay scope %lu phase%u pc%o\n",(unsigned long)n,phase,s.words[5]);return 2;}cm_priority_display_dump(n,phase,status,&s,&io);if(status!=EXEC_OK)break;}}return 0;}
