/* SPDX-License-Identifier: MIT */
#ifndef AGC_HARDWARE_DIVIDE_H
#define AGC_HARDWARE_DIVIDE_H
#include "agc_multiword.h"
/* Fixed-point hardware-DV arithmetic on two full16-bit one's-complement
 * register words (L applies its built-in overflow correction) and an already-resolved full16-bit divisor. Preserve all
 * four dividend/divisor zero-sign combinations, mixed limb signs and raw
 * quotient-overflow recurrence results. No mathematical saturation policy.
 * The caller resolves any A/L/Z register operand alias before this call.
 * No CPU state, instruction decoding, memory, dispatch or floating point. */
agc_register_pair agc_hardware_divide(agc_register_pair dividend,agc_register divisor);
#endif
