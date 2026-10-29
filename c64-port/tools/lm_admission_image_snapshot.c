/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_admission_image_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_ADMISSION_IMAGE_CASES;++n){apollo_overlay_state image;p66_bias_io io;exec_address job=lm_admission_image_input(&image,&io,n);enum exec_status status=lm_admission_image(&image,&io,(enum lm_admission_entry)(n%4u),job);unsigned i;printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
