/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_alarm_image_cases.h"
#include "cm_fixed_compat.h"
#include "cm_alarm_rope_probe.h"
#include "cm_alarm_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ALARM_IMAGE_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_word literal=0;uint16_t address;enum exec_status result;cm_alarm_image_input(&s,&io,n);if(cm_alarm_fixed_address(&s,&io,&address)&&!cm_fixed_compat_read(address,&literal))return 2;result=cm_alarm_image(&s,&io,(enum cm_alarm_record_entry)(n%4u),literal);if(result!=EXEC_OK){fprintf(stderr,"alarmimage scope %lu\n",(unsigned long)n);return 2;}cm_alarm_image_dump(n,result,&s,&io);}return 0;}
