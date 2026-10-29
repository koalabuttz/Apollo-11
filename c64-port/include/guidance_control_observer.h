/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_CONTROL_OBSERVER_H
#define GUIDANCE_CONTROL_OBSERVER_H
#include "guidance_norm.h"
/* Control debris observer only; ordinary 15-bit limb inputs. No arithmetic
 * output, memory write, instruction decoding, or dispatch is performed.
 * Callers retain incoming Q on unsupported phases and on alarms. */
agc_word p66_ddv_return_q(agc_word incoming_q,agc_dp dividend,agc_dp divisor);
agc_word p66_acceleration_return_q(uint8_t phase,agc_word incoming_q,
    agc_dp incoming_mpac,agc_dp desired_rate,agc_dp taurod,
    const p66_norm_scratch *after,agc_word after_mptemp);
#endif
