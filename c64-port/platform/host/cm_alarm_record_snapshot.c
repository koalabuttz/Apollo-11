/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_alarm_record_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ALARM_RECORD_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_word literal=cm_alarm_record_input(&s,&io,n);enum exec_status status=cm_alarm_record(&s,&io,(enum cm_alarm_record_entry)(n%4u),literal);if(status!=EXEC_OK)return 2;cm_fixed_dump(n,0,status,&s,&io);}return 0;}
