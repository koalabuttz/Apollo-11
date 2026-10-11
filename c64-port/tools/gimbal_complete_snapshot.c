/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "gimbal_complete_cases.h"
#include "gimbal_complete.h"
static void emit(uint16_t w){putchar(w&255u);putchar(w>>8);}
int main(int argc,char **argv){static apollo_overlay_state s;gimbal_io io;uint16_t n,i,ok;
 int input=argc==2 && strcmp(argv[1],"--input")==0;
 for(n=0;n<GIMBAL_COMPLETE_CASE_COUNT;++n){gimbal_complete_prepare(&s,&io,n);
#ifdef INPUT_ONLY
 (void)input;ok=1;
#else
 ok=input?1:gimbal_complete(&s,&io,gimbal_complete_mission(n));
#endif
 emit(n);emit(ok);for(i=0;i<2048u;++i)emit(s.words[i]);emit(io.superbank);emit(io.interrupt_enabled);}
 return ferror(stdout)?2:0;}
