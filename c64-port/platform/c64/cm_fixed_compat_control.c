/* SPDX-License-Identifier: MIT */
#include "cm_fixed_compat_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){enum exec_status status;
 if(selector<2u)status=cm_fixed_caller_enter(s,CM_FIXED_IO,(uint8_t)selector);
 else if(selector==2u)status=cm_supdacal_data(s,CM_FIXED_IO,*CM_FIXED_FETCH);
 else return 1;
 return status!=EXEC_OK;
}
