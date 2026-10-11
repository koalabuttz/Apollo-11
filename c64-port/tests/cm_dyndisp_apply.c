/* SPDX-License-Identifier: MIT */
#include "cm_dyndisp_cases.h"
uint8_t cm_dyndisp_apply(apollo_overlay_state *s,cm_native_io *io,uint8_t checkpoint){return cm_dyndisp_step(s,io,checkpoint);}
