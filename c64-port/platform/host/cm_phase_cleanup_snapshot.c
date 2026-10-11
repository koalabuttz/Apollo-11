/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_phase_cleanup_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_PHASE_CLEANUP_CASES;++n){apollo_overlay_state s;cm_native_io io;enum exec_status status;cm_phase_cleanup_input(&s,&io,n);status=cm_phase_cleanup(&s,&io);if(status!=EXEC_OK)return 2;cm_fixed_dump(n,0,status,&s,&io);}return 0;}
