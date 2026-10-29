/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_scan_exception_cases.h"
int main(void){uint16_t n;for(n=0;n<LM_SCAN_EXCEPTION_CASES;++n){apollo_overlay_state m;p66_bias_io io;unsigned i;enum exec_status st;lm_scan_exception_input(&m,&io,n);st=lm_scan_exception(&m,&io,(enum lm_scan_entry)(n%4u));printf("%u %u",n,(unsigned)st);for(i=0;i<2048u;++i)printf(" %u",m.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
