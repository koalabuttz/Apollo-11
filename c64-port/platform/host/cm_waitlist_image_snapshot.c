/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_waitlist_image_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_WLIMAGE_CASES;++n){enum cm_wl_status status;cm_waitlist_image_input(&s,&io,(uint16_t)n);status=cm_waitlist_image(&s,&io,(exec_address){s.words[2]==03277?03260:s.words[2]==02576?02612:s.words[0705],s.words[2]==03277?050067:s.words[2]==02576?050066:s.words[0706]});printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
