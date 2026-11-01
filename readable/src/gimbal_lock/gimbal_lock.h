/* SPDX-License-Identifier: MIT */
/* GIMBAL_LOCK_AVOIDANCE.agc, both missions' (Luminary099 p. 364,
 * Comanche055 pp. 412-413): LOCSKIRT, where KALCMANU picks the maneuver's
 * rate and time and starts the steering. All interpretive code, one table
 * each, run by the readable interpreter; the binding is
 * src/binding/gimbal_lock_routines.c. */
#ifndef READABLE_GIMBAL_LOCK_H
#define READABLE_GIMBAL_LOCK_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code NOGIMLOC_CODE;
#else
extern const struct interpretive_code WCALC_CODE;
#endif

#endif
