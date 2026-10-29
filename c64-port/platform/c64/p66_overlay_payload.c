/* SPDX-License-Identifier: MIT */
#include "p66_native.h"
/* Five independent builds select one reviewed source continuation group through
 * P66_NATIVE_ONLY_GROUP. State is the shared physical erasable image; there is
 * no private persistent guidance snapshot or interpreter opcode dispatch. */
uint16_t apollo_overlay_main(apollo_overlay_state *state,uint16_t selector) {
    return p66_native_entry(state,selector);
}
