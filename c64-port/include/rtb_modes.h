/* SPDX-License-Identifier: MIT */
#ifndef RTB_MODES_H
#define RTB_MODES_H
#include "rtb_angles.h"
/* RTB DPMODE (RT8_OP_CODES): CAF ZERO; TCF SLOAD2 +2, which zeroes MPAC +2
 * and MODE and goes on to DANZIG. Entered as RTB leaves it (Z at DPMODE,
 * FB its bank); anything else is RTB_OUT_OF_SCOPE with the image unchanged. */
enum rtb_status rtb_dpmode(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission);
#endif
