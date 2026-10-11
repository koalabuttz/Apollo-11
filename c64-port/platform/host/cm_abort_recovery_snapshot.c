/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_abort_recovery_cases.h"
#include "cm_abort_recovery_dump.h"
int main(void){uint32_t n;uint8_t phase;for(n=0;n<CM_ABORT_RECOVERY_CASES;++n){apollo_overlay_state s;cm_native_io io;cm_abort_recovery_input(&s,&io,n);for(phase=0;phase<(n&1u?1u:7u);++phase){enum exec_status status=cm_abort_recovery_stage(&s,&io,phase);if(status!=EXEC_OK)return 2;cm_abort_recovery_dump(n,phase,(uint16_t)status,&s,&io);}}return 0;}
