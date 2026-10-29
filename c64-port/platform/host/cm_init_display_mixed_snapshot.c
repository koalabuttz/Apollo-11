/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_init_display_mixed_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;uint16_t phase;static apollo_overlay_state s;static cm_native_io io;exec_transfer next;
 for(n=0;n<CM_INIT_DISPLAY_MIXED_CASES;++n){cm_init_display_mixed_input(&s,&io,n);for(phase=0;phase<3u;++phase){enum exec_status status=cm_init_display_mixed_stage(&s,&io,(uint8_t)phase,&next);if(status!=EXEC_OK||next.address.bank!=(agc_word)(s.words[6]|io.superbank)||next.address.location!=(next.interpretive?s.words[0164]:s.words[5]))return 2;cm_fixed_dump(n,phase,status,&s,&io);if(phase==1u&&((n/2u)%7u)!=0u)break;}}
 return 0;}
