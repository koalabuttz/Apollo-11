/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_DAPS_H
#define CM_TVC_DAPS_H
#include "agc_image_ops.h"
/* Comanche055 TVCDAPS (bank 20, 2530-3500): the PITCHDAP and YAWDAP T5
 * interrupt tasks (CDU rate estimate, body-rate error, three-cascade
 * forward filter, offset, actuator limit, TVC command and precompensation),
 * the DAPINIT T5 task, and the PCOPY/YCOPY filter-copy cycles FINCOPY
 * reaches by ISWCALL, compiled one original word at a time over the
 * canonical 2048-word image. Channel 14 (WOR, TVC drive bits 11 and 12) is
 * held in the io record.
 * Entry: Z names PITCHDAP (2530), YAWDAP (2732), DAPINIT (3107), a HACK
 * return (2541, 2543, 2743, 2745) or PCOPY+1/YCOPY+1 (2675, 3051, which end
 * in TC Q and need the caller's return in Q, 4000-7777); FB bank 20, EB E6
 * (BB coherent), any raw A/L, erasable contents and channel 14; otherwise
 * CM_TVC_DAPS_OUT_OF_SCOPE, image and io unchanged. CM_TVC_DAPS_OK leaves Z
 * at RESUME, NOQRSM, HACK (Q the TC's return) or the caller's return. */
enum cm_tvc_daps_status {
    CM_TVC_DAPS_OK = 0,
    CM_TVC_DAPS_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel14; } cm_tvc_daps_io;
enum cm_tvc_daps_status cm_tvc_daps(agc_image *, cm_tvc_daps_io *);
#endif
