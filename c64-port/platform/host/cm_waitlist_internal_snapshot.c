/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_waitlist_internal_cases.h"
#include "../../tools/cm_alarm2_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_WAITLIST_INTERNAL_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_word literal=cm_waitlist_internal_input(&s,&io,n);enum cm_wl_status status=cm_waitlist_internal(&s,&io,(enum cm_waitlist_internal_entry)(n/8192u),literal);if(status!=CM_WL_OK&&status!=CM_WL_FULL&&status!=CM_WL_SOURCE_OVERFLOW)return 2;cm_alarm2_image_dump(n,(uint16_t)status,&s,&io);}return 0;}
