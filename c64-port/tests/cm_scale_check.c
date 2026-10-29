/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_scale_check.h"
#include "cm_scale_vectors.h"
uint16_t cm_scale_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0,number;cm_native_io io,old;
 if(n>=CM_SCALE_TARGET_COUNT)return 1;
 number=cm_scale_numbers[n];cm_scale_input(s,&io,number);memcpy(before,s,sizeof *s);old=io;
 if(cm_scale_output(s,&io,(enum dsky_scale_entry)(number%3u))!=cm_scale_results[n])++failures;
 patch=cm_scale_offsets[n];end=cm_scale_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;
  if(patch<end&&cm_scale_patch[patch][0]==f)expected=cm_scale_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;

 return failures;
}
