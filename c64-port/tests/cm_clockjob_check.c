/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_clockjob_check.h"
#include "cm_clockjob_vectors.h"
uint16_t cm_clockjob_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0;cm_native_io io,old;if(n>=64u)return 1;cm_clockjob_input(s,&io,cm_clockjob_numbers[n]);memcpy(before,s,sizeof *s);old=io;if(cm_clockjob_apply(s,&io,cm_clockjob_numbers[n])!=cm_clockjob_results[n])++failures;patch=cm_clockjob_offsets[n];end=cm_clockjob_offsets[n+1u];for(f=0;f<2050u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;if(patch<end&&cm_clockjob_patch[patch][0]==f)expected=cm_clockjob_patch[patch++][1];if(actual!=expected)++failures;}if(patch!=end)++failures;old.superbank=io.superbank;old.interrupt_enabled=io.interrupt_enabled;if(memcmp(&old,&io,sizeof io))++failures;return failures;}
