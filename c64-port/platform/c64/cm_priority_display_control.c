/* SPDX-License-Identifier: MIT */
#include "cm_priority_display_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){enum exec_status status=cm_priority_display_step(s,CM_PRIORITY_DISPLAY_IO,selector);CM_PRIORITY_DISPLAY_FIXTURE->status=(uint16_t)status;return status==EXEC_OUT_OF_SCOPE;}
