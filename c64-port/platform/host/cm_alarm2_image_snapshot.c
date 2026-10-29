/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_alarm2_image_cases.h"
#include "cm_fixed_compat.h"
#include "cm_alarm_rope_probe.h"
#include "cm_alarm2_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ALARM2_IMAGE_CASES;++n){apollo_overlay_state s;cm_native_io io;uint16_t address;agc_word word=0;enum exec_status status;cm_alarm2_image_input(&s,&io,n);if(s.words[5]==03261u&&cm_alarm2_producer(&s,&io)!=EXEC_OK)return 2;if(cm_alarm2_fixed_address(&s,&io,&address)&&!cm_fixed_compat_read(address,&word))return 2;status=cm_alarm2_image(&s,&io,word);if(status!=EXEC_OK)return 2;cm_alarm2_image_dump(n,status,&s,&io);}return 0;}
