/* SPDX-License-Identifier: MIT */
/* The programs the kalcmanu-steering cases run: each mission's
 * KALCMANU_STEERING routines alone, entered at NEWOPS. IRC_LM_ONLY
 * or IRC_CM_ONLY builds one mission (the C64 diagnostics). */
#include "agc_transfer.h"
const agc_native_program *irc_program(uint8_t cm);
#ifndef IRC_CM_ONLY
#include "lm_kalcmanu_steering.h"
static const agc_native_program lm={&lm_kalcmanu_steering_basic,1,&lm_kalcmanu_steering_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
#endif
#ifndef IRC_LM_ONLY
#include "cm_kalcmanu_steering.h"
static const agc_native_program cm={&cm_kalcmanu_steering_basic,1,&cm_kalcmanu_steering_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
#endif
#if defined IRC_LM_ONLY
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
#elif defined IRC_CM_ONLY
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
#else
const agc_native_program *irc_program(uint8_t m){return m?&cm:&lm;}
#endif
