/* SPDX-License-Identifier: MIT */
#include "lm_context_lifecycle_cases.h"
#include <stdio.h>
int main(void){unsigned n,i;for(n=0;n<LM_CONTEXT_LIFECYCLE_CASES;++n){
 apollo_overlay_state image;p66_bias_io io;agc_word channel;exec_transfer next;
 enum exec_status status;lm_context_lifecycle_input(&image,&io,&channel,(uint16_t)n);
 status=lm_context_lifecycle(&image,&io,&channel,(enum lm_context_lifecycle_entry)(n&1u),&next);
 if(status!=EXEC_OK&&status!=EXEC_IDLE){fprintf(stderr,"case%u unsupported%u\n",n,status);return 2;}
 printf("%u %u %u %u %u",n,(unsigned)status,(unsigned)next.interpretive,
  (unsigned)next.address.location,(unsigned)next.address.bank);
 for(i=0;i<2048u;++i)printf(" %u",image.words[i]);
 printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,channel);
 }return 0;}
