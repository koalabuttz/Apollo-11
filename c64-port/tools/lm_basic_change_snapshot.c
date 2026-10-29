/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_basic_change_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_BASIC_CHANGE_CASES;++n){apollo_overlay_state image;p66_bias_io io;exec_transfer next;unsigned i;lm_basic_change_input(&image,&io,n);if(lm_basic_change(&image,&io,(enum lm_basic_change_entry)(n&1u),&next)!=EXEC_OK)return 2;printf("%u %u %u %u",n,next.address.location,next.address.bank,next.interpretive);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
