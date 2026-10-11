/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cm_idle_recheck_cases.h"
static void reject(unsigned index,agc_word value){apollo_overlay_state s,before;cm_native_io io,oldio;exec_transfer next={{0123,0},0},oldnext;cm_idle_recheck_input(&s,&io,0u);s.words[index]=value;before=s;oldio=io;oldnext=next;assert(cm_idle_recheck(&s,&io,&next)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io)&&!memcmp(&next,&oldnext,sizeof next));}
int main(void){reject(0067,077777);reject(0067,1);reject(0164,2);reject(0167,0);reject(0167,040000);reject(3,1);reject(7,1);assert(cm_idle_recheck(0,0,0)==EXEC_OUT_OF_SCOPE);puts("PASS: invalid priority/compiled caller/resource contracts preserve all canonical objects");return 0;}
