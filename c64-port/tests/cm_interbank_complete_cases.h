/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_COMPLETE_CASES_H
#define CM_INTERBANK_COMPLETE_CASES_H
#include "cm_interbank_complete.h"
#define CM_INTERBANK_COMPLETE_BASE_CASES UINT32_C(917504)
#define CM_INTERBANK_COMPLETE_CASES UINT32_C(921088)
static inline unsigned cm_interbank_complete_kind(uint32_t n){return (unsigned)(n<CM_INTERBANK_COMPLETE_BASE_CASES?n/65536u:(n-CM_INTERBANK_COMPLETE_BASE_CASES)/256u);}
agc_word cm_interbank_complete_input(apollo_overlay_state *,cm_native_io *,uint32_t);
#endif
