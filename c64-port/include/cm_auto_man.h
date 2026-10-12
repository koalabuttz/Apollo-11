/* SPDX-License-Identifier: MIT */
#ifndef CM_AUTO_MAN_H
#define CM_AUTO_MAN_H
#include "agc_image_ops.h"
/* Comanche055 AUTOMATIC_MANEUVERS (bank 21, 3250-3764): the RCS DAP's
 * attitude-hold, free-drive and rate-damping logic (AHFNOROT), the
 * phase-plane jet timing for all three axes (JETS) and the T6PROG hand-off
 * to JETSLECT, compiled one original word at a time over the canonical
 * 2048-word image. Channels 31 and 32 are read only.
 * Entry: FB bank 21, EB E6 (BB coherent), any raw A/L/Q and erasable
 * contents and channels, and Z one of
 *   3250 AHFNOROT, 3441 JETS, 3757 T6PROG (TCF from RCSATT's T5PHASE2);
 *   3304 (STICKCHK return; RMANNDX, PMANNDX, YMANNDX 0..3);
 *   3732, 3734 (SMALLMP returns in JTIME; SPNDX 0..2).
 * Otherwise CM_AUTO_MAN_OUT_OF_SCOPE, image unchanged. CM_AUTO_MAN_OK
 * leaves Z at RESUME, REINIT, T6PROGM, or STICKCHK/SMALLMP with Q the
 * TC's return. */
enum cm_auto_man_status {
    CM_AUTO_MAN_OK = 0,
    CM_AUTO_MAN_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel31, channel32; } cm_auto_man_io;
enum cm_auto_man_status cm_auto_man(agc_image *, const cm_auto_man_io *);
#endif
