/* SPDX-License-Identifier: MIT */
#include "native_module_registry.h"
#include "native_registry_table.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector) {
 (void)image;
 return native_registry_copy(native_registry_modules,NATIVE_REGISTRY_MODULES,
  (native_registry_lease *)(uintptr_t)selector);
}
