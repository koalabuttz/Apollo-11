/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERBANK_CASES_H
#define LM_INTERBANK_CASES_H
#include "lm_interbank.h"
#define LM_INTERBANK_BASE_CASES UINT32_C(917504)
#define LM_INTERBANK_CASES UINT32_C(921088)
static inline unsigned lm_interbank_kind(uint32_t n){return (unsigned)(n<LM_INTERBANK_BASE_CASES?n/65536u:(n-LM_INTERBANK_BASE_CASES)/256u);}
agc_word lm_interbank_input(apollo_overlay_state *,p66_bias_io *,uint32_t);
#endif
