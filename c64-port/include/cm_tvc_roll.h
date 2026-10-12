/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_ROLL_H
#define CM_TVC_ROLL_H
#include "agc_image_ops.h"
/* Comanche055 TVCROLLDAP (bank 16, 3272-3717): the ROLLDAP waitlist task
 * (rate estimate, phase-plane switching logic, jet firing time, roll jet
 * selection) and the NOROLL1 T6 task, compiled one original word at a time
 * over the canonical 2048-word image. Channels 6 (WRITE, roll jets) and 13
 * (WOR, T6 enable) are held in the io record.
 * Entry: Z names ROLLDAP (3272) or NOROLL1 (3676), FB bank 16, EB E6 (BB
 * coherent), any raw A/L/Q, erasable contents and channels; otherwise
 * CM_TVC_ROLL_OUT_OF_SCOPE, image and io unchanged. CM_TVC_ROLL_OK leaves
 * Z at TASKOVER or NOQRSM, the file's only exits. */
enum cm_tvc_roll_status {
    CM_TVC_ROLL_OK = 0,
    CM_TVC_ROLL_OUT_OF_SCOPE = 1
};
typedef struct { agc_word channel6, channel13; } cm_tvc_roll_io;
enum cm_tvc_roll_status cm_tvc_roll(agc_image *, cm_tvc_roll_io *);
#endif
