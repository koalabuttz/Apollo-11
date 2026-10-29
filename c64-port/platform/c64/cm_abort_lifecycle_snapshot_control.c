/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_lifecycle_fixture *f=CM_LIFECYCLE_FIXTURE;enum exec_status status;if(selector<=4u)status=cm_abort_snapshot_stage(s,CM_LIFECYCLE_IO,(uint8_t)selector,f->literal);else if(selector==12u)status=cm_abort_whimper(s,CM_LIFECYCLE_IO,&f->in_isr);else return 1;f->status=(uint16_t)status;return status!=EXEC_OK;}
