/* SPDX-License-Identifier: MIT */
#ifndef DSKY_READ_PAIR_H
#define DSKY_READ_PAIR_H
#include "dsky_nv_admission.h"
/* Original READLO, fresh ordinary pair without TPAGREE. Normal MIXBR1 uses
 * NOUNADD and retained EB; mixed MIXBR2 uses IDAD1TEM[DECOUNT] and selects
 * its ECADR bank. Ordinary pairphysical0100..3776, no bank crossing; row0..2,
 * coherentbank40/anyEB/selectingSUPER/legalIRQ, ordinarycallerQ0100..7777,
 * rawA/L and ordinaryRAM. Stops before actual saved caller instruction.
 * Canonical image owns scratch. Producer exclusion/nonaliasing IO required.
 * Unsupported input preserves image/IO. No counter/device reads or decoder. */
enum p66_bias_status dsky_read_pair(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission);
#endif
