/* SPDX-License-Identifier: MIT */
#ifndef CM_JET_SELECT_H
#define CM_JET_SELECT_H
#include "agc_image_ops.h"
/* Comanche055 JET_SELECTION_LOGIC (bank 17, 2552-3736): the JETSLECT T5
 * task (translation-controller and failure-aware jet words for pitch, yaw
 * and roll, jet counts, firing times and the T6 sequence) and the T6START
 * T6 task that writes the jet words to channels 5 and 6, compiled one
 * original word at a time over the canonical 2048-word image. Channels 5,
 * 6 and 13 are written; channel 31 is read.
 * Entry: FB bank 17, EB E6 (BB coherent), any raw A/L/Q, erasable contents
 * and channels, and Z one of
 *   2552 JETSLECT (T5RUPT via JETADDR; XTRANS -4..+4, the source of the
 *     translation index CCS XTRANS stores);
 *   3617 T6START (T6RUPT via T6ADDR; BLAST+1, BLAST1+1, BLAST2+1 each 0,
 *     4 or 11, the REPLACER/REPLACEP/REPLACEY offsets T6SETUP stores).
 * Otherwise CM_JET_SELECT_OUT_OF_SCOPE, image and io unchanged.
 * CM_JET_SELECT_OK leaves Z at RESUME. CM_JET_SELECT_FAULT (an INDEX
 * outside the file's tables) cannot occur from an admitted entry. */
enum cm_jet_select_status {
    CM_JET_SELECT_OK = 0,
    CM_JET_SELECT_OUT_OF_SCOPE = 1,
    CM_JET_SELECT_FAULT = 2
};
typedef struct { agc_word channel5, channel6, channel13, channel31; } cm_jet_select_io;
enum cm_jet_select_status cm_jet_select(agc_image *, cm_jet_select_io *);
#endif
