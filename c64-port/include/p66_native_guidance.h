/* SPDX-License-Identifier: MIT */
#ifndef P66_NATIVE_GUIDANCE_H
#define P66_NATIVE_GUIDANCE_H
#include "p66_native.h"
enum p66_guidance_group {P66_GUIDANCE_UNIT=6,P66_GUIDANCE_ACCELERATION=7,
 P66_GUIDANCE_DOT=8,P66_GUIDANCE_STORE=9,P66_GUIDANCE_AFC=10};
#define P66_GUIDANCE_MAPPING_COUNT 48u
#define P66_GUIDANCE_ALARM_1302 01302u
#define P66_GUIDANCE_UNIT_TERMINAL 0x8003u
/* Explicit compiled groups with one authoritative image. The caller excludes
 * source producers. Successful entries commit all owned source scratch/VAC;
 * a SQRT abort commits the verified pre-SQRTABRT state and reports1302. */
uint16_t p66_native_guidance_entry(apollo_overlay_state *,uint16_t);
/* Extends the immutable prefix mappings with a resident constant table.
 * Replaces bounded UNIT entry, adds real radial/acceleration and three
 * projection safe points, then stops before feedback at03434. The caller
 * owns binding lifetime and performs source exclusion on initialization.
 * Configure binding.invoke for explicit compiled numerical selectors; null selects
 * directly linked host kernels. No pointers into overlay code may persist. */
extern const exec_context_mapping p66_native_guidance_mappings[P66_GUIDANCE_MAPPING_COUNT];
enum exec_status p66_native_guidance_init(exec_native *,executive *,p66_native_binding *);
#endif
