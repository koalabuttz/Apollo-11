/* SPDX-License-Identifier: MIT */
/* What the routine bindings share: pointing the machine (src/agc/agc.h) at
 * the flight program's state, and the keys of its routine tables. Plumbing
 * only (README.md, src/binding/). */
#ifndef READABLE_BINDING_H
#define READABLE_BINDING_H

#include "resident_kernel.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#endif
#include "../agc/agc.h"

/* Point agc at the program's memory and channels. False if any is missing. */
bool bind_machine(const agc_native_program *program, apollo_overlay_state *state, void *device);

/* Keys: physical addresses of fixed-fixed (04000-07777) and bank 01. */
#define FF(z)        (uint16_t)(2048u + ((z) - 04000u))
#define B1(z)        (uint16_t)(1024u + ((z) - 02000u))
#define COUNT(table) (uint16_t)(sizeof(table) / sizeof(table)[0])

#endif
