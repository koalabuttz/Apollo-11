/* SPDX-License-Identifier: MIT */
#ifndef GEOMETRY_RTB_H
#define GEOMETRY_RTB_H
#include "overlay_runtime.h"
#include "p66_bias.h"
/* Comanche CSM_GEOMETRY's SXTLOGIC (23,2027), which SXTNB's RTB runs, as
 * the original does from the RTB's transfer (Z at the routine, FB 23):
 * the 19.775-degree trunnion offset added to MPAC, SHORTMP by 1/4, then
 * TC DANZIG. Returns RTB_OK with Z at DANZIG and Q the word after the TC,
 * or RTB_OUT_OF_SCOPE when Z, FB or the image is not one the routine
 * runs on. */
uint8_t geometry_rtb(apollo_overlay_state *, p66_bias_io *);
/* Comanche SHORTMP (07256 through DMPSUB2): the TP fraction in MPAC times
 * the single precision word in A, left in MPAC; Q as its TC Q leaves it.
 * The caller's TC has set Q and continues at the word after it.
 * RTB_OUT_OF_SCOPE when A or the product's TS would skip on overflow. */
uint8_t geometry_shortmp(apollo_overlay_state *);
#endif
