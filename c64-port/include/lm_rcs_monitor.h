/* SPDX-License-Identifier: MIT */
#ifndef LM_RCS_MONITOR_H
#define LM_RCS_MONITOR_H
#include "p66_bias.h"
#include "executive.h"
/* RCSMON/RCSMONIT entry 06,3156, despite the nominal BANK23 directive.
 * The canonical image contains ordinary 15-bit words (raw 16-bit A/L).
 * Channel32 is an explicit stable inverted input; PVALVEST is 0..0377.
 * Caller owns interrupt exclusion (IRQ0), coherent EB/BB, REGZERO0,
 * and the Executive/mask/input producers. SUPER retains bits0160 only.
 * Core priorities cannot be +0. NEWJOB is -0 or a valid displacement;
 * first-free-core0 with NEWJOB+0 is excluded because source calls CCSHOLE.
 * These exclusions avoid inconsistent initialized Executive states.
 * NOVAC queues actual 1/ACCJOB 02454/040106 at priority027000.
 * Success stops before external RESUME05270, resource exhaustion before
 * BAILOUT105716 (status EXEC_NO_CORE). This API executes neither external
 * continuation nor the admitted ACCJOB. IRQ/SUPER/channel32 are retained.
 * No Executive safe point occurs inside this basic interrupt routine. */
enum exec_status lm_rcs_monitor(apollo_overlay_state *,p66_bias_io *,agc_word channel32);
/* Exact physical fixed-word accessor for two inline CADR words and16 table
 * words; unsupported address/null output returns0 without writing output. */
uint8_t lm_rcs_monitor_constant_read(uint16_t physical_fixedword,uint16_t *);
#endif
