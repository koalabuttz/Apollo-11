/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_priolarm_cases.h"
#include "cm_priolarm_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_PRIOLARM_CASES;++n){apollo_overlay_state s;cm_native_io io;enum cm_priolarm_result status;cm_priolarm_input(&s,&io,n);status=cm_priolarm(&s,&io);if(status==CM_PRIOLARM_OUT_OF_SCOPE)return 2;cm_priolarm_dump(n,(uint16_t)status,&s,&io);}return 0;}
