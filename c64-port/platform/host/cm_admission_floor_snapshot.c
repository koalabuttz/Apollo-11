/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_admission_floor_cases.h"
#include "../../tools/cm_alarm2_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ADMISSION_FLOOR_CASES;++n){apollo_overlay_state s;cm_native_io io;exec_address job;enum exec_status status;job=cm_admission_floor_input(&s,&io,n);status=cm_admission_floor(&s,&io,(enum cm_admission_entry)(n%4u),job);if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return 2;cm_alarm2_image_dump(n,(uint16_t)status,&s,&io);}return 0;}
