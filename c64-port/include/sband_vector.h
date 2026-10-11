/* SPDX-License-Identifier: MIT */
#ifndef SBAND_VECTOR_H
#define SBAND_VECTOR_H
#include "overlay_runtime.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Native original VXV body, entry through pending DANZIG, no polling or
 * external callee execution. Exclusive canonical image ownership is required.
 * ADDRWD binds six ordinary erasable words; each product rereads actual RAM,
 * including scratch aliases. Producers remain excluded across this call.
 * Invalid mission, banks, vector MODE, or data descriptor preserves the image.
 * SUPER/IRQ are external and untouched. A/L are raw16; ordinary RAM is15-bit.
 */
uint8_t sband_vector_cross(apollo_overlay_state *image,
                          enum apollo_fixed_mission mission);
uint8_t sband_vector_projection(apollo_overlay_state *image,
                               enum apollo_fixed_mission mission);
/* Fixed entry: fixed[0..5] are the immutable words at source ADDRWD..+5.
 * The caller supplies real bank-selected source data; no fixed ROM is cached
 * in canonical erasable RAM. The array must not alias the mutable image.
 * Ordinary descriptors02000..03772 or04000..07772; no bank boundary crossing.
 * Producers remain excluded and the source FB/SUPER selection is unchanged. */
uint8_t sband_vector_cross_fixed(apollo_overlay_state*,enum apollo_fixed_mission,const agc_word fixed[6]);
uint8_t sband_vector_projection_fixed(apollo_overlay_state*,enum apollo_fixed_mission,const agc_word fixed[6]);
#endif
