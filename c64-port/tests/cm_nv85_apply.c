/* SPDX-License-Identifier: MIT */
#include "cm_nv85_cases.h"
uint8_t cm_nv85_apply(apollo_overlay_state *s,cm_native_io *io,uint8_t checkpoint){return cm_nv85_step(s,io,checkpoint);}
