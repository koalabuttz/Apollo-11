/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_abort_word_cases.h"
#include "cm_fixed_compat.h"
#include "cm_abort_word_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ABORT_WORD_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_word literal=0;uint16_t address;enum exec_status result;cm_abort_word_input(&s,&io,n);if(cm_abort_word_fixed_address(&s,&io,&address)&&!cm_fixed_compat_read(address,&literal))return 2;result=cm_abort_word(&s,&io,literal);if(result!=EXEC_OK){fprintf(stderr,"alarmimage scope %lu\n",(unsigned long)n);return 2;}cm_abort_word_dump(n,result,&s,&io);}return 0;}
