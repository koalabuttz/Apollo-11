/* SPDX-License-Identifier: MIT */
#include "dsky_fixed_display.h"
#include "lm_fixed_compat.h"
#include "cm_fixed_compat.h"
bool dsky_fixed_display_data(enum dsky_nv_mission mission,uint16_t address,agc_word *out,const void *user)
{(void)user;if(mission==DSKY_NV_LM)return lm_fixed_compat_read(address,out);if(mission==DSKY_NV_CM)return cm_fixed_compat_read(address,out);return false;}
