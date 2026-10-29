/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_super_dispatch_cases.h"
#include "../../tools/cm_alarm2_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_SUPER_DISPATCH_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_transfer next;cm_super_dispatch_input(&s,&io,n);if(cm_super_dispatch(&s,&io,&next)!=EXEC_OK)return 2;cm_alarm2_image_dump(n,EXEC_OK,&s,&io);}return 0;}
