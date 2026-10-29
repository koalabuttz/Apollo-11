/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_abort_lifecycle_cases.h"
#include "cm_abort_lifecycle_dump.h"
int main(void){uint32_t n;uint8_t phase;for(n=0;n<CM_ABORT_LIFECYCLE_CASES;++n){apollo_overlay_state s;cm_native_io io;uint8_t in_isr;agc_word literal=cm_abort_lifecycle_input(&s,&io,n,&in_isr);for(phase=0;phase<cm_abort_lifecycle_count(n);++phase){uint8_t group=cm_abort_lifecycle_group(n,phase);uint16_t result;if(!cm_abort_lifecycle_step(&s,&io,group,literal,&in_isr,&result)){fprintf(stderr,"native lifecycle reject case%lu phase%u group%u PC%05o\n",(unsigned long)n,phase,group,s.words[5]);return 2;}cm_abort_lifecycle_dump(n,phase,result,&s,&io,in_isr);}}return 0;}
