/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_STROKE_H
#define CM_TVC_STROKE_H
#include "agc_image_ops.h"
/* Comanche055 TVCSTROKETEST (bank 20, 3501-3611): STRKTSTI/TSTINIT/STRKCHK
 * (the V68 job) and the HACK waveform generator with its HACKWLST waitlist
 * task, compiled one original word at a time over the canonical 2048-word
 * image. Channel 14 (WOR) and the interrupt enable (INHINT) are held in the
 * io record.
 * Entry: Z names an entry, FB bank 20, EB E6 (BB coherent). HACK, its
 * WAITLIST return and HACKWLST need CARD -4..-1, the values TSTINIT and the
 * waveform leave there (CARD indexes the FCARD table). Anything else is
 * CM_TVC_STROKE_OUT_OF_SCOPE, image and io unchanged. CM_TVC_STROKE_OK
 * leaves Z at the first transfer out of the file: ENDOFJOB, WAITLIST, or
 * BUNKER (TC BUNKER runs the word held there: the DAP's return saved by
 * QXCH, or TC TASKOVER). */
enum cm_tvc_stroke_status {
    CM_TVC_STROKE_OK = 0,
    CM_TVC_STROKE_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel14, interrupt_enabled; } cm_tvc_stroke_io;
enum cm_tvc_stroke_status cm_tvc_stroke(agc_image *, cm_tvc_stroke_io *);
#endif
