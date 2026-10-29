/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_internal_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_wl_internal_fixture *f=CM_WL_INTERNAL_FIXTURE;if(selector)return 1;f->status=(uint16_t)cm_waitlist_internal(s,CM_WL_INTERNAL_IO,(enum cm_waitlist_internal_entry)(f->source_case/8192u),f->literal);return f->status!=CM_WL_OK&&f->status!=CM_WL_FULL&&f->status!=CM_WL_SOURCE_OVERFLOW;}
