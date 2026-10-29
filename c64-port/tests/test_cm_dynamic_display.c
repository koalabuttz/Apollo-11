/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_dynamic_display_cases.h"
static apollo_overlay_state s,before;
int main(void){cm_native_io io,old;uint16_t n,i,base,failures=0;uint8_t p;for(n=0;n<CM_DYNAMIC_DISPLAY_CASES;++n){cm_dynamic_display_input(&s,&io,n);base=s.words[0120];for(p=0;p<53u;++p)if(!cm_dynamic_display_step(&s,&io,p)){++failures;break;}if(s.words[base-1u]!=(agc_word)(base-1u)||s.words[0167]!=077777u||s.words[5]!=03231u||s.words[4]!=02000u||io.interrupt_enabled!=1u||s.words[01001]!=6u||s.words[01002]!=85u)++failures;for(i=1;i<7u;++i)if(s.words[0167u+i*12u]!=077777u)++failures;before=s;old=io;if(cm_dynamic_display_step(&s,&io,53)||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}if(cm_dynamic_display_step(0,&io,0)||cm_dynamic_display_step(&s,0,0))++failures;printf("64 actual DYNDISP/normal N85/END paths: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
