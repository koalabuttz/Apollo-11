/* SPDX-License-Identifier: MIT */
#include "cm_scan_zero_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_scan_fixture *f=CM_SCAN_FIXTURE;enum exec_status status;
 if(selector!=0u)return 1;status=cm_scan_zero(s,CM_SCAN_IO,&f->next);f->status=(uint16_t)status;return status!=EXEC_OK&&status!=EXEC_IDLE;
}
