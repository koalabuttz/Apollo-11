/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_waitlist_image_cases.h"
int main(void){uint16_t n;for(n=0;n<64u;++n){unsigned i;apollo_overlay_state image;p66_bias_io io;uint16_t number=(uint16_t)((n*257u+29u)%LM_WAITLIST_IMAGE_CASES);exec_address job=lm_waitlist_image_input(&image,&io,number);printf("%u %u %u %u",number,(unsigned)(number&1u),job.location,job.bank);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
