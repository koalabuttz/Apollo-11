/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_admission_image_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_ADMISSION_CASES;++n){enum exec_status status;cm_admission_image_input(&s,&io,(uint16_t)n);status=cm_admission_image(&s,&io,(exec_address){s.words[0705],s.words[0706]},(uint8_t)(n<512u));printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
