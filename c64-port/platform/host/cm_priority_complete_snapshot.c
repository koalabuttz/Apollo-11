/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_priority_complete_cases.h"
#include "../../tools/cm_fixed_compat_dump.h"
static apollo_overlay_state image;static cm_native_io io;
int main(void){uint32_t n;for(n=0;n<CM_PRIORITY_COMPLETE_CASES;++n){exec_transfer next;enum exec_status status;cm_priority_complete_input(&image,&io,n);status=cm_priority_complete(&image,&io,&next);if(status!=EXEC_OK||next.address.location!=(next.interpretive?image.words[0164]:image.words[5])||next.address.bank!=(image.words[6]|io.superbank))return 2;cm_fixed_dump(n,(uint16_t)(n&1u),(unsigned)status,&image,&io);}return 0;}
