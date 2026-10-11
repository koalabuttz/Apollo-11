/* SPDX-License-Identifier: MIT */
#include "cm_init_display_mixed_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_init_mixed_fixture *f=CM_INIT_MIXED_FIXTURE;if(selector!=f->phase||selector>2u)return 1;f->status=(uint16_t)cm_init_display_mixed_stage(s,CM_INIT_MIXED_IO,(uint8_t)selector,&f->next);return f->status!=EXEC_OK;}
