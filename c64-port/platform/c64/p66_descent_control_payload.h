/* SPDX-License-Identifier: MIT */
#ifndef C64_P66_DESCENT_CONTROL_PAYLOAD_H
#define C64_P66_DESCENT_CONTROL_PAYLOAD_H
#include "p66_bias.h"
#include "agc_word.h"
/* Resident argument descriptor, passed through the existing explicit-selector
 * word. Module identity selects one compiled service. These external hardware
 * words are never assigned invented AGC erasable addresses. The descriptor and
 * its targets remain resident until the module returns through the C ABI. */
typedef struct {p66_bias_io *io;agc_word *channel14;} p66_descent_control_arguments;
#endif
