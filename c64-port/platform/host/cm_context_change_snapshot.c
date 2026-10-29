/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_context_change_cases.h"
#include "../../tools/cm_fixed_compat_dump.h"
static apollo_overlay_state image;static cm_native_io io;
int main(void){uint32_t n;for(n=0;n<CM_CONTEXT_CHANGE_CASES;++n){exec_transfer next;enum exec_status status;cm_context_change_input(&image,&io,n);status=cm_context_change(&image,&io,(uint8_t)(n&1u),&next);if(status!=EXEC_OK||next.interpretive!=(n/2u)%2u||next.address.location!=(next.interpretive?image.words[0164]:image.words[5])||next.address.bank!=(image.words[6]|io.superbank))return 2;cm_fixed_dump(n,(uint16_t)(n&1u),(unsigned)status,&image,&io);}return 0;}
