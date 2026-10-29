/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_interbank_cases.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){unsigned n,i;for(n=0;n<CM_INTERBANK_CASES;++n){agc_word literal=cm_interbank_input(&s,&io,(uint16_t)n);enum exec_status status=cm_interbank(&s,&io,(enum cm_interbank_entry)(n/512u),literal);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
