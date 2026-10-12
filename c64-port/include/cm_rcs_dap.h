/* SPDX-License-Identifier: MIT */
#ifndef CM_RCS_DAP_H
#define CM_RCS_DAP_H
#include "agc_image_ops.h"
/* Comanche055 RCS-CSM_DIGITAL_AUTOPILOT (bank 21, 2105-3247): the RCS DAP
 * T5 cycle (RCSATT and its restart REDORCS: initialisation, rate filter,
 * Kalman gains, CDU increments, FDAI needles, hand controller and manual
 * rate commands) and the routines other files call (NEEDLER, NEEDLE11,
 * NEEDLES, ZEROJET, STICKCHK, REINIT, T6PROGM), compiled one original word
 * at a time over the canonical 2048-word image. Channels 12 (RAND, WAND,
 * WOR), 13 (WOR) and 14 (WOR) are held in the io record; channel 31 is
 * read only.
 * Entry: FB bank 21, EB E6 (BB coherent), any raw A/L, erasable contents
 * and channels, and Z one of
 *   2105 REDORCS, 2122 RCSATT (ATTKALMN -1..11, RATEINDX 0..6, the
 *     Kalman-gain and manual-rate table indices);
 *   2236 (SMALLMP return; SPNDX 0..2, DPNDX = 2 SPNDX, ATTKALMN -1..11),
 *   2401 (DPADD return; SPNDX 0..2, DPNDX = 2 SPNDX),
 *   2360, 2616 (NOVAC returns), 2550, 2552 (IBNKCALL returns),
 *   2534 REINIT, 3067 T6PROGM;
 *   2420 NEEDLER, 2443 NEEDLE11, 2476 NEEDLES, 2632 ZEROJET, 3130 STICKCHK
 *     (each ends in TC Q: Q a caller's return, 3250-3764 or 4000-7777).
 * Otherwise CM_RCS_DAP_OUT_OF_SCOPE, image and io unchanged.
 * CM_RCS_DAP_OK leaves Z at RESUME, IBNKCALL, NOVAC, SMALLMP, DPADD (Q
 * the TC's return), JETS, T6PROG, AHFNOROT, or the caller's return. */
enum cm_rcs_dap_status {
    CM_RCS_DAP_OK = 0,
    CM_RCS_DAP_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel12, channel13, channel14, channel31; } cm_rcs_dap_io;
enum cm_rcs_dap_status cm_rcs_dap(agc_image *, cm_rcs_dap_io *);
#endif
