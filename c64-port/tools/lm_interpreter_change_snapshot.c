/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_interpreter_change_cases.h"
static apollo_overlay_state state;
int main(void){unsigned n,i;for(n=0;n<LM_INTERPRETER_CHANGE_CASES;++n){p66_bias_io io;exec_transfer next;enum exec_status status;lm_interpreter_change_input(&state,&io,(uint16_t)n);status=lm_interpreter_change(&state,&io,&next);printf("%u %u",n,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",state.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,lm_interpreter_change_channel11((uint16_t)n),lm_interpreter_change_channel14((uint16_t)n));}return 0;}
