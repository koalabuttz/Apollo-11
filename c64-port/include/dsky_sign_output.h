/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SIGN_OUTPUT_H
#define DSKY_SIGN_OUTPUT_H
#include "dsky_decimal_general.h"
enum dsky_sign_output_route {DSKY_SIGN_CALLER,DSKY_SIGN_CCSHOLE};
/* Actual source +ON/-ON for data rows: DSPCOUNT0..14, ordinary callerQ,
 * coherent bank40/anyEB/selectingSUPER/legalIRQ/ordinaryRAM, rawA/L.
 * Stops actual caller or before selected clean-relay CCSHOLE body.
 * Canonical image owns all scratch; caller excludes producers and supplies
 * nonnull distinct objects. Invalid declared control inputs unchanged. */
enum p66_bias_status dsky_sign_output(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,uint8_t negative,enum dsky_sign_output_route *);
#endif
