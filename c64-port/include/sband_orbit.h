/* SPDX-License-Identifier: MIT */
#ifndef SBAND_ORBIT_H
#define SBAND_ORBIT_H
#include "sband_control.h"
/* Actual returned-conic antenna caller through Earth UNIT/MXV body entry or
 * the true lunar LUNPOS interpreted handoff. Explicit source controls only.
 * Requires exact returned PC/LOC/BANKSET, coherent original bank042 (octal),
 * NEWJOB+0, ZERO0, EDOP0, five valid VAC bases and ordinary RAM/Q/raw16 A/L.
 * Both X2 zero representations select Earth, other ordinary words select Moon.
 * Producers remain excluded throughout each call. SUPER/IRQ are externally
 * owned and retained. Unsupported contexts preserve complete image/latches.
 * This caller does not execute UNIT, MXV or the orbital callee bodies. */
bool sband_orbit(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
