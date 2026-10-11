/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_abort_fixture *f=CM_ABORT_FIXTURE;if(selector!=f->phase||selector>4u)return 1;f->status=(uint16_t)cm_abort_snapshot_stage(s,CM_ABORT_IO,(uint8_t)selector,f->literal);return f->status!=EXEC_OK;}
