/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_delay_image_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_DELAY_IMAGE_CASES;++n){apollo_overlay_state s;p66_bias_io io;agc_word delay=lm_delay_image_input(&s,&io,n);unsigned i;enum wl_status status=n%3u==2u?lm_waitlist_internal(&s,&io):lm_delay_image(&s,&io,(enum lm_delay_entry)(n&1u),delay);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
