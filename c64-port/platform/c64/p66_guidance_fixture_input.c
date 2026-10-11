/* SPDX-License-Identifier: MIT */
#include "p66_guidance_fixture.h"
#include "p66_guidance_live_cases.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_guidance_fixture *f=P66_GUIDANCE_FIXTURE;uint8_t mixed=(uint8_t)(selector>>8);selector&=255;
 if(selector>=2||mixed>1)return 1;
 /* Sole source image persists. This leased view is discarded on ABI return. */
 pgl_prepare(image,(executive *)(uintptr_t)0xc100,selector);
 if(mixed)pgl_prepare_high(image,selector);
 f->number=(uint8_t)selector;f->mixed=mixed;f->stage=0;f->failures=0;return 0;
}
