/* SPDX-License-Identifier: MIT */
#include "p66_native_guidance.h"
uint16_t apollo_overlay_main(apollo_overlay_state *state,uint16_t selector){return p66_native_guidance_entry(state,selector);}
