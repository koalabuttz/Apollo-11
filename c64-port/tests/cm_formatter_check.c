/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_formatter_check.h"
#include "cm_formatter_vectors.h"
uint16_t cm_formatter_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0,number;cm_native_io io,old;
 if(n>=CM_FORMATTER_TARGET_COUNT)return 1;
 number=cm_formatter_numbers[n];cm_formatter_input(s,&io,number);memcpy(before,s,sizeof *s);old=io;
 if(cm_formatter_run(s,&io,number)!=cm_formatter_results[n])++failures;
 patch=cm_formatter_offsets[n];end=cm_formatter_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;
  if(patch<end&&cm_formatter_patch[patch][0]==f)expected=cm_formatter_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;

 return failures;
}
