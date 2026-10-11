/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_interbank_check.h"
#include "cm_interbank_cases.h"
static apollo_overlay_state s,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_interbank_check();agc_word literal=cm_interbank_input(&s,&io,0);before=s;oldio=io;if(cm_interbank(&s,&io,(enum cm_interbank_entry)14,literal)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io))++failures;printf("7168 original CM interbank full states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
