/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_display_check.h"
#include "cm_display_vectors.h"
uint16_t cm_display_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *before){uint16_t f,patch,end,failures=0,number;cm_native_io io,old;exec_address next;
 if(n>=CM_DISPLAY_TARGET_COUNT)return 1;
 number=cm_display_numbers[n];cm_display_input(s,&io,number);memcpy(before,s,sizeof *s);old=io;
 if(cm_display_run(s,&io,number,&next)!=cm_display_results[n])++failures;
 patch=cm_display_offsets[n];end=cm_display_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){uint16_t expected=f<2048u?before->words[f]:f==2048u?old.superbank:old.interrupt_enabled;uint16_t actual=f<2048u?s->words[f]:f==2048u?io.superbank:io.interrupt_enabled;
  if(patch<end&&cm_display_patch[patch][0]==f)expected=cm_display_patch[patch++][1];
  if(actual!=expected)++failures;
 }
 if(patch!=end)++failures;
 if(next.location!=s->words[5]||next.bank!=(uint16_t)(s->words[6]|io.superbank))++failures;
 return failures;
}
