/* SPDX-License-Identifier: MIT */
#ifndef CM_NATIVE_IMAGE_H
#define CM_NATIVE_IMAGE_H
#include "overlay_runtime.h"
#include "cm_phase.h"
#include "cm_enema.h"
/* External device latches and native fault metadata, not erasable memory.
 * Application owns one such metadata record and one 2048-word flight image.
 * Gather/commit views are transient. Commit before any overlay return; regather
 * after dispatcher or source group boundaries because scratch aliases overlap.
 * No callback/module pointer is stored in the flight word image. */
typedef struct {
 agc_word channel11,channel12,channel13,channel14;
 agc_word main_keys,navigation_keys,superbank,interrupt_enabled;
 agc_word trap31a,trap31b,trap32,executive_alarm,waitlist_alarm;
 /* Lane G: the RCS jet output channels 5 and 6 and the input channels
  * 30-33, for the DAP, TVC and entry routines. */
 agc_word channel05,channel06,channel30,channel31,channel32,channel33;
} cm_native_io;
void cm_native_gather_executive(const apollo_overlay_state *,const cm_native_io *,cm_executive *);
void cm_native_commit_executive(apollo_overlay_state *,cm_native_io *,const cm_executive *);
void cm_native_gather_waitlist(const apollo_overlay_state *,const cm_native_io *,cm_waitlist *);
void cm_native_commit_waitlist(apollo_overlay_state *,cm_native_io *,const cm_waitlist *);
void cm_native_gather_restart(const apollo_overlay_state *,const cm_native_io *,cm_restart_state *);
void cm_native_commit_restart(apollo_overlay_state *,cm_native_io *,const cm_restart_state *);
void cm_native_gather_phase(const apollo_overlay_state *,const cm_native_io *,cm_phase_state *);
void cm_native_commit_phase(apollo_overlay_state *,cm_native_io *,const cm_phase_state *);
void cm_native_gather_enema_io(const apollo_overlay_state *,const cm_native_io *,cm_enema_io *);
void cm_native_commit_enema_io(apollo_overlay_state *,cm_native_io *,const cm_enema_io *);
#endif
