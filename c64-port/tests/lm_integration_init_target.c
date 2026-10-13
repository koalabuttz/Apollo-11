/* SPDX-License-Identifier: MIT */
/* The program the lm-integration-init cases run: Luminary099 INTEGRATION_INITIALIZATION's
 * routines alone, entered at NEWOPS or at their basic address. */
#include "agc_transfer.h"
#include "lm_integration_init.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_integration_init_basic,1,&lm_integration_init_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
