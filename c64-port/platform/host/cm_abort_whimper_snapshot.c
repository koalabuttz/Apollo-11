/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_abort_whimper_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ABORT_WHIMPER_CASES;++n){apollo_overlay_state s;cm_native_io io;uint8_t in_isr;enum exec_status status;cm_abort_whimper_input(&s,&io,&in_isr,n);status=cm_abort_whimper(&s,&io,&in_isr);if(status!=EXEC_OK||in_isr)return 2;cm_fixed_dump(n,0,status,&s,&io);}return 0;}
