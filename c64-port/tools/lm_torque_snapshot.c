/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_torque_cases.h"
static void emit(uint16_t w){putchar(w&255u);putchar(w>>8);}
int main(int argc,char **argv){static apollo_overlay_state s;lm_torque_io io;uint16_t n,i,ok;
 int input=argc==2 && strcmp(argv[1],"--input")==0;
 for(n=0;n<LM_TORQUE_CASE_COUNT;++n){lm_torque_prepare(&s,&io,n);ok=input?1:lm_torque(&s,&io);
 emit(n);emit(ok);for(i=0;i<APOLLO_SHARED_WORDS;++i)emit(s.words[i]);emit(io.superbank);emit(io.interrupt_enabled);}
 return ferror(stdout)?2:0;}
