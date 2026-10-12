/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_RESTARTS_H
#define CM_TVC_RESTARTS_H
#include "agc_image_ops.h"
/* Comanche055 TVCRESTARTS (bank 16, 3140-3271): the TVC restart package
 * REDOTVC, the T5 outcounter tasks ENABL2 and CMDSOUT, the CHKSTRK return
 * from SWICHOVR, the FINCOPY+3 return from a copycycle, and the EXRSTRT
 * waitlist task, compiled one original word at a time over the canonical
 * 2048-word image. Channels 12 and 14 (output latches written by WOR) are
 * held outside erasable memory in the io record.
 * Entry: Z names an entry, FB bank 16, EB E6 (BB coherent). EXRSTRT needs
 * TVCEXPHS 1..6 (an index into its restart table); REDOTVC with TVCEXPHS
 * +0 and an odd positive TVCPHASE needs TVCPHASE <= 9 (an index into
 * TVCCADR). Anything else is CM_TVC_RESTARTS_OUT_OF_SCOPE, image and io
 * unchanged. CM_TVC_RESTARTS_OK leaves Z at the first transfer out of the
 * file (WAITLIST, RESUME, NOQRSM, POSTJUMP, IBNKCALL, ISWCALL or a
 * TVCEXECUTIVE restart point) with Q as the transfer leaves it. */
enum cm_tvc_restarts_status {
    CM_TVC_RESTARTS_OK = 0,
    CM_TVC_RESTARTS_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel12, channel14; } cm_tvc_restarts_io;
enum cm_tvc_restarts_status cm_tvc_restarts(agc_image *, cm_tvc_restarts_io *);
#endif
