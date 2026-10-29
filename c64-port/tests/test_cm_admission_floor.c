/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cm_admission_floor_cases.h"
static void reject(unsigned field,agc_word value){apollo_overlay_state s,before;cm_native_io io,oldio;exec_address job=cm_admission_floor_input(&s,&io,1);s.words[field]=value;before=s;oldio=io;assert(cm_admission_floor(&s,&io,CM_ADMIT_FINDVAC,job)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));}
int main(void){apollo_overlay_state s,before;cm_native_io io,oldio;exec_address job;reject(0,01000);reject(2,1);reject(3,1);reject(7,1);job=cm_admission_floor_input(&s,&io,2);io.interrupt_enabled=1;before=s;oldio=io;assert(cm_admission_floor(&s,&io,CM_ADMIT_SPVAC,job)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));assert(cm_admission_floor(0,&io,CM_ADMIT_NOVAC,job)==EXEC_OUT_OF_SCOPE);puts("PASS: bare-priority admission rejected unsupported producers without changing canonical image/metadata");return 0;}
