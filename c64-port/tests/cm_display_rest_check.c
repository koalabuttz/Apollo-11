/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_display_rest_check.h"
#include "cm_display_rest_vectors.h"
uint8_t cm_display_rest_checkpoint_input(uint8_t n,apollo_overlay_state *s,cm_native_io *io){if(n>=60u)return 0;cm_display_rest_input(s,io,cm_display_rest_numbers[n]);return 1;}
uint16_t cm_display_rest_checkpoint(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){cm_native_io io,old;uint16_t f,p,end,failures=0;if(!cm_display_rest_checkpoint_input(n,s,&io))return 1;*before=*s;old=io;if((unsigned)cm_display_rest(s,&io)!=(unsigned)EXEC_OK)++failures;p=cm_display_rest_after_offsets[n];end=cm_display_rest_after_offsets[n+1u];for(f=0;f<2050u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;if(p<end&&cm_display_rest_after_patch[p][0]==f)expected=cm_display_rest_after_patch[p++][1];if(actual!=expected)++failures;}if(p!=end)++failures;old.superbank=io.superbank;old.interrupt_enabled=io.interrupt_enabled;if(memcmp(&old,&io,sizeof io))++failures;return failures;}
