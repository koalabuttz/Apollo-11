/* SPDX-License-Identifier: MIT */
#ifndef INTERPRETED_SQUARE_H
#define INTERPRETED_SQUARE_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
enum interpreted_square_mission {INTERPRETED_SQUARE_LM=0,INTERPRETED_SQUARE_CM=1};
enum interpreted_square_kind {INTERPRETED_DSQ=0,INTERPRETED_DSQSUB=1};
enum interpreted_square_status {INTERPRETED_SQUARE_OK=0,INTERPRETED_SQUARE_OUT_OF_SCOPE=1};
/* Original DSQ/DSQSUB entries3174/3300 in bank0. DSQ ends before source
 * DANZIG6061/6030; DSQSUB completes TCQ at saved fixed Q with Q3. No source
 * dispatcher/Executive check is performed. Ordinary MPAC words, coherent
 * EB/BB, FB0 and REGZERO0 required; MODE/OVFIND/IRQ retained unchanged.
 * Exclude shared input/scratch producers until the actual source return. */
enum interpreted_square_status interpreted_square(apollo_overlay_state *,
 enum interpreted_square_mission,enum interpreted_square_kind);
#endif
