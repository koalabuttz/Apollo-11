/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_vac5_store_cases.h"
#include "cm_fixed_compat_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_VAC5_STORE_CASES;++n){apollo_overlay_state s;cm_native_io io;enum exec_status status;cm_vac5_store_input(&s,&io,n);status=cm_vac5_store(&s,&io);if(status!=EXEC_OK)return 2;cm_fixed_dump(n,0,status,&s,&io);}return 0;}
