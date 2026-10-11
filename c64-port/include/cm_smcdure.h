/* SPDX-License-Identifier: MIT */
#ifndef CM_SMCDURE_H
#define CM_SMCDURE_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum cm_smcdure_phase {CM_SMCDURE_LOAD_FIRST=0,CM_SMCDURE_FIRST_COSINE,
CM_SMCDURE_PUSH_LOAD_LAST,CM_SMCDURE_LAST_SINE,CM_SMCDURE_SUBTRACT_LAST,
CM_SMCDURE_DIVIDE_MIDDLE_COSINE,CM_SMCDURE_STORE_FIRST,CM_SMCDURE_MIDDLE_SINE,
CM_SMCDURE_SCALE_FIRST,CM_SMCDURE_SUBTRACT_MIDDLE,CM_SMCDURE_STORE_LOAD_FIRST,
CM_SMCDURE_FIRST_SINE,CM_SMCDURE_PUSH_LOAD_LAST_AGAIN,CM_SMCDURE_LAST_COSINE,
CM_SMCDURE_ADD_LAST,CM_SMCDURE_SCALE_LAST,CM_SMCDURE_STORE_LAST,CM_SMCDURE_RETURN};
enum cm_smcdure_status {CM_SMCDURE_OK=0,CM_SMCDURE_OUT_OF_SCOPE=1};
/* Original CM SMCDURES23,3742. Fixed native phase selection compiles the18
 * source operations; it never reads or decodes an AGC instruction word.
 * All phases commit the canonical source state. EDOP=0 DANZIG points are
 * genuine Executive checks; nonzero EDOP retains pending right operation.
 * DTHETASM03211..03216 and source trig arrays are stable caller inputs.
 * FIXLOC names an owned43-word VAC, PUSHLOC starts atbase and returns there.
 * Caller excludes producers throughout each phase; scheduling is external. */
enum cm_smcdure_status cm_smcdure(apollo_overlay_state *,enum cm_smcdure_phase);
#endif
