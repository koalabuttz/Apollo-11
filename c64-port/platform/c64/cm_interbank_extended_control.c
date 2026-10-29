/* SPDX-License-Identifier: MIT */
#include "cm_interbank_extended_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_ib_ext_fixture *f=CM_IB_EXT_FIXTURE;enum exec_status status;
 if(selector>=14u)return 1;
 status=cm_interbank_extended(s,CM_IB_EXT_IO,(enum cm_interbank_entry)selector,f->literal,f->fetched);f->status=(uint16_t)status;
 return status!=EXEC_OK&&status!=EXEC_OUT_OF_SCOPE;
}
