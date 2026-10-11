/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_abort_snapshot_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ABORT_SNAPSHOT_CASES;++n){apollo_overlay_state s;cm_native_io io;agc_word literal=cm_abort_snapshot_input(&s,&io,n);unsigned phase;for(phase=0;phase<5u;++phase){enum exec_status status=cm_abort_snapshot_stage(&s,&io,(uint8_t)phase,literal);if(status!=EXEC_OK)return 2;cm_fixed_dump(n,(uint16_t)phase,status,&s,&io);}}return 0;}
