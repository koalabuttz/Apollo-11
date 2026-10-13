/* SPDX-License-Identifier: MIT */
#ifndef LM_FINDCDUW_RTB_H
#define LM_FINDCDUW_RTB_H
#include "rtb_angles.h"
/* The two RTB routines FINDCDUW's PAUTNO reaches (RTB NORMUNIT; RTB RTB
 * NORMUNIT QUICTRIG), entered as lmi_rtb leaves them (Z the routine's
 * first word, FB its bank, Q = SWRETURN) and run to DANZIG 06061 through
 * the existing native bodies:
 *   NORMUNIT 10,3726: rtb_vector's NORMUNIT front, then UNIT+1
 *            (rtb_unit's three phases) to DANZIG;
 *   QUICTRIG 23,3615: lm_powered_quick to its TC A, the return to
 *            SWRETURN, then SWRETURN to DANZIG.
 * RTB_OUT_OF_SCOPE when a body refuses, or at UNIT's alarm 1302 (a zero
 * vector, which INITCDUW's unit vectors and the guidance's thrust and
 * window vectors never are). */
enum rtb_status lm_findcduw_normunit(apollo_overlay_state *,p66_bias_io *);
enum rtb_status lm_findcduw_quictrig(apollo_overlay_state *,p66_bias_io *);
#endif
