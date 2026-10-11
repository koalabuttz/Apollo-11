/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_TRIG_H
#define CM_POWERED_TRIG_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum cm_powered_trig_status { CM_POWERED_TRIG_OK=0,CM_POWERED_TRIG_OUT_OF_SCOPE=1 };
/* Original CM CDUTRIGS23,3507 / CD*TR*GS23,3515 and canonical internal
 * boundaries. start live=1 reads source CDU registers; live=0 uses CDUSPOT.
 * Calls start; cosine; sine; next; repeat cosine/sine/next twice.
 * The nine math boundaries are source DANZIG06030 with EDOP physical1
 * (pending EXIT), so none is an Executive handoff. Caller retains ownership
 * and excludes producers until the basic return, PC=oldQ/Q0143. No private
 * continuation state, AGC opcode decoder, or inferred source job check.
 * Requires bank23/consistentEB/BB, ZERO0, ordinary MODE and fixed-code source
 * saved return02000..07777. IRQ remains caller-owned and unchanged. */
enum cm_powered_trig_status cm_powered_cdu_start(apollo_overlay_state *,uint8_t live);
enum cm_powered_trig_status cm_powered_cdu_cosine(apollo_overlay_state *);
enum cm_powered_trig_status cm_powered_cdu_sine(apollo_overlay_state *);
enum cm_powered_trig_status cm_powered_cdu_next(apollo_overlay_state *);
#endif
