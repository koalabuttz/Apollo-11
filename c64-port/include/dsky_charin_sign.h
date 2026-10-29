/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_SIGN_H
#define DSKY_CHARIN_SIGN_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
/* Actual CHARIN +/- through SIGNTEST, sign relay writes and FIXCLPAS to
 * real ENDOFJOB. Bank40/SUPER100 anyEB, ordinary DSPCOUNT/DECBRNCH/CLPASS.
 * Accepted signs require nonzero relay words; unsupported unchanged. Caller
 * excludes producers. LM/CM literal return addresses remain independently
 * qualified; no source opcode or code-word dispatch is implemented. */
enum p66_bias_status dsky_charin_sign(apollo_overlay_state *,dsky_charin_io *);
enum exec_status cm_charin_sign(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
