/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_fixed_compat_cases.h"
#include "cm_fixed_compat.h"
#include "../../tools/cm_fixed_compat_dump.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){uint32_t n;for(n=0;n<CM_FIXED_COMPAT_CASES;++n){uint16_t address;agc_word word;enum exec_status status;cm_fixed_compat_input(&s,&io,n);status=cm_fixed_caller_enter(&s,&io,(uint8_t)(n>=40960u));if(status!=EXEC_OK)return 2;cm_fixed_dump(n,0,(unsigned)status,&s,&io);if(cm_supdacal_address(&s,&io,&address)!=EXEC_OK||address!=(uint16_t)(n%40960u)||!cm_fixed_compat_read(address,&word))return 2;status=cm_supdacal_data(&s,&io,word);if(status!=EXEC_OK)return 2;cm_fixed_dump(n,1,(unsigned)status,&s,&io);}return 0;}
