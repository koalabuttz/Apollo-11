/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_admission_zero_cases.h"
#include "../../tools/cm_fixed_compat_dump.h"
static apollo_overlay_state image;static cm_native_io io;
int main(void){uint32_t n;for(n=0;n<CM_ADMISSION_ZERO_CASES;++n){exec_address job=cm_admission_zero_input(&image,&io,n);enum exec_status status=cm_admission_zero(&image,&io,(enum cm_admission_entry)(n%4u),job);if(status!=EXEC_OK&&status!=EXEC_NO_CORE&&status!=EXEC_NO_VAC)return 2;cm_fixed_dump(n,(uint16_t)(n%4u),(unsigned)status,&image,&io);}return 0;}
