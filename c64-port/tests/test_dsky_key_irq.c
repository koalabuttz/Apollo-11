/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_key_irq_check.h"
static apollo_overlay_state state,before;
int main(void){uint8_t n;uint16_t failures=0;dsky_key_irq_io io={{0,0},1};
 for(n=0;n<DSKY_KEY_IRQ_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_key_irq_check(n,&state,&before));
 dsky_key_irq_prepare(&state,0);state.words[4]=02000;memcpy(&before,&state,sizeof state);
 if(dsky_key_irq(&state,&io,0)!=EXEC_OUT_OF_SCOPE || memcmp(&state,&before,sizeof state)!=0)++failures;
 printf("DSKY key IRQ: 64 complete2051-field original images, %u failures\n",failures);return failures!=0;
}
