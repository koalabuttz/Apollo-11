/* SPDX-License-Identifier: MIT */
#include "p66_native_descent.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 return p66_native_descent_entry(image,selector);
}
