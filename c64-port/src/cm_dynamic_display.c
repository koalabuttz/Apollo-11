/* SPDX-License-Identifier: MIT */
#include "cm_dynamic_display.h"
#include "cm_dyndisp.h"
#include "cm_nv85.h"
uint8_t cm_dynamic_display_step(apollo_overlay_state *s,cm_native_io *io,uint8_t route){if(s==0||io==0)return 0;if(route<27u)return cm_dyndisp_step(s,io,route);if(route<53u)return cm_nv85_step(s,io,(uint8_t)(route-27u));return 0;}
