/* SPDX-License-Identifier: MIT; passive native data payload, never a decoder. */
#include "cm_fixed_compat.h"
#include "overlay_runtime.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){agc_word word;(void)s;if(!cm_fixed_compat_read(selector,&word))return UINT16_C(0177777);return word;}
