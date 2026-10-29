/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_waitlist_image_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_WAITLIST_IMAGE_CASES;++n){apollo_overlay_state image;p66_bias_io io;uint16_t i;exec_address task=lm_waitlist_image_input(&image,&io,n);enum wl_status status=lm_waitlist_image(&image,&io,(enum lm_waitlist_entry)(n&1u),task);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
