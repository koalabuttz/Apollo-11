/* SPDX-License-Identifier: MIT */
#ifndef RTB_PULSE_RETURN_H
#define RTB_PULSE_RETURN_H
#include "rtb_angles.h"
/* Original trailing PULSEIMU TCF DANZIG after actual external IMUPULSE bank
 * return. Nonnull canonical image/io, ordinary15bit memory/raw16 A/L,
 * coherent original containing bank10 LM/bank22 CM and excluded producers.
 * Preserves source IRQ/SUPER/pending continuation; L is source normalized.
 * Does not execute DANZIG job check or external IMUPULSE itself. */
enum rtb_status rtb_pulse_return(apollo_overlay_state *,p66_bias_io *,enum rtb_mission);
#endif
