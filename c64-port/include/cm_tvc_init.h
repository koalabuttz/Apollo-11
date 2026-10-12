/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_INIT_H
#define CM_TVC_INIT_H
#include "agc_image_ops.h"
/* Comanche055 TVCINITIALIZE (bank 17, 2147-2551): TVCDAPON/MRCLEAN,
 * TVCINIT1-4 with ATTINIT and NEEDLEIN, the DAPINIT T5 hand-off, the V46
 * CSM/LM switch-over (PRESWTCH, SWICHOVR, MCLEANJR), LOADCOEF and S40.15,
 * compiled one original word at a time over the canonical 2048-word image.
 * Channel 11 (read by RAND) and the interrupt enable (INHINT in SWICHOVR)
 * are held in the io record.
 * Entry: Z names an entry, FB bank 17, EB E6 (BB coherent); S40.15, entered
 * by IBNKCALL and left by TC Q, needs Q 4000-7777 (a fixed-fixed return).
 * Anything else is CM_TVC_INIT_OUT_OF_SCOPE, image and io unchanged.
 * CM_TVC_INIT_OK leaves Z at the first transfer out of the file (RESUME,
 * IBNKCALL, WAITLIST, POSTJUMP, TC RTRNLOC, the caller's return from
 * S40.15, or TC 1703 when TS VARK overflows into the CSMN10 data) with Q as
 * the transfer leaves it. */
enum cm_tvc_init_status {
    CM_TVC_INIT_OK = 0,
    CM_TVC_INIT_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel11, interrupt_enabled; } cm_tvc_init_io;
enum cm_tvc_init_status cm_tvc_init(agc_image *, cm_tvc_init_io *);
#endif
