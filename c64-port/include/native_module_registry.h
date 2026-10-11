/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_MODULE_REGISTRY_H
#define NATIVE_MODULE_REGISTRY_H
#include "overlay_runtime.h"
#include "irq_delivery.h"
#define NATIVE_REGISTRY_NAME_BYTES 17u
#define NATIVE_REGISTRY_LEASE_ADDRESS 0xc600u
#define NATIVE_REGISTRY_LEASE_BYTES 64u
/* Ordinary native metadata only. Complete descriptor/name live outside the
 * replaceable slot. requested is an explicit native module ID, not an opcode. */
typedef struct {
 apollo_overlay_module module;
 char filename[NATIVE_REGISTRY_NAME_BYTES];
 uint16_t requested;
 uint8_t valid;
} native_registry_lease;
typedef struct {
 apollo_overlay_runtime *loader;
 const apollo_overlay_module *bootstrap;
 native_registry_lease *lease;
 const exec_exclusion *exclusion;
 uint8_t busy;
} native_module_registry;
/* Pure bounded lookup, used by the metadata payload. No source image/I/O is
 * touched. Failure invalidates the output so an earlier record cannot escape. */
enum apollo_overlay_status native_registry_copy(const apollo_overlay_module *,
 uint8_t count,native_registry_lease *);
/* bootstrap, its filename, loader, lease and exclusion remain resident. The
 * caller owns nonoverlapping initialized storage; target lease is c600..c63f.
 * Initialization performs no I/O and rejects missing producer exclusion. */
enum apollo_overlay_status native_registry_init(native_module_registry *,
 apollo_overlay_runtime *,const apollo_overlay_module *,native_registry_lease *,
 const exec_exclusion *);
/* Lookup and actual stage load share one exclusion token. Every metadata C
 * frame returns before loading the stage. All failures retain caller result;
 * no pointer into the metadata slot survives. No lookup changes source state. */
enum apollo_overlay_status native_registry_lookup(native_module_registry *,uint16_t);
enum apollo_overlay_status native_registry_call(native_module_registry *,uint16_t,
 uint16_t selector,uint16_t *result);
#endif
