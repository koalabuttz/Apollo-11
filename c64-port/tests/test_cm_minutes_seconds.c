/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_minutes_seconds_check.h"
static apollo_overlay_state s,before;
int main(void){cm_native_io io,old;uint16_t n,failures=0;uint8_t phase,g;for(n=0;n<112u;++n)failures=(uint16_t)(failures+cm_ms_checkpoint_check((uint8_t)n,&s,&before));for(n=0;n<7u;++n)for(g=0;g<9u;++g){phase=cm_ms_checkpoint_input((uint8_t)(n?63u+n:0u),&s,&io);switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.interrupt_enabled=2;break;case 6:io.superbank=0;break;case 7:phase=6;break;default:if(phase==254u||phase<=2u||phase==5u)s.words[0777]=2;else s.words[01023]=0;break;}before=s;old=io;if(phase==254u){if(cm_decimal_pair(&s,&io)==EXEC_OK)++failures;}else if(cm_minutes_seconds_phase(&s,&io,phase))++failures;if(memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("112 full CM signed decimal/M-S states and 63 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
