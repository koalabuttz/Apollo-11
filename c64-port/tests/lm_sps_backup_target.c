/* SPDX-License-Identifier: MIT */
/* The program the lm-sps-backup cases run: Luminary099 SPS_BACK-UP_RCS_CONTROL's routines
 * alone (basic code). */
#include "agc_transfer.h"
#include "lm_sps_backup.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_sps_backup_basic,1,&lm_sps_backup_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
