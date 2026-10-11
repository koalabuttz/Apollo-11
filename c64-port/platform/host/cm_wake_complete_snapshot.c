/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_wake_complete_cases.h"
#include "../../tools/cm_alarm2_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_WAKE_COMPLETE_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_transfer next;enum exec_status status;cm_wake_complete_input(&s,&io,n);status=cm_wake_complete(&s,&io,&next);if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND)return 2;cm_alarm2_image_dump(n,status,&s,&io);}return 0;}
