/* SPDX-License-Identifier: MIT
 * Links an unchanged ABI 1 body (apollo_overlay_main) as an ABI 2 module: the
 * body's result becomes frame->value and the module returns DONE. */
#include "overlay_v2.h"
uint16_t apollo_overlay_main(apollo_overlay_state *,uint16_t);
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector) {
    f->value=apollo_overlay_main(f->state,selector);
    return APOLLO_OV2_DONE;
}
