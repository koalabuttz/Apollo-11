/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cm_waitlist_internal_cases.h"
static void reject(unsigned index,agc_word value){apollo_overlay_state s,before;cm_native_io io,oldio;agc_word literal=cm_waitlist_internal_input(&s,&io,0);s.words[index]=value;before=s;oldio=io;assert(cm_waitlist_internal(&s,&io,CM_WL_DLY2_MINUS1,literal)==CM_WL_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));}
int main(void){reject(2,0);reject(2,0177777);reject(0,2);reject(1,0177777);reject(061,077);reject(7,1);assert(cm_waitlist_internal(0,0,CM_WL_FIXDELAY,1)==CM_WL_OUT_OF_SCOPE);puts("PASS: internal WAITLIST invalid caller/delay/task/bank contracts unchanged");return 0;}
