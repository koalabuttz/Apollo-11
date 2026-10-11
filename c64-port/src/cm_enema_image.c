/* SPDX-License-Identifier: MIT */
#include "cm_enema_image.h"
typedef struct {cm_executive e;cm_waitlist w;cm_restart_state r;cm_phase_state p;cm_enema_io cpu;} cm_enema_views;
enum cm_enema_result cm_enema_image(apollo_overlay_state *s,cm_native_io *io){
#ifdef __mos__
 /* Reset-on-load transient cache: every field is regathered before use. Avoid
  * the toolchain's large automatic-frame epilogue defect on this source group. */
 static cm_enema_views views;
#else
 cm_enema_views views;
#endif
 enum cm_enema_result status;
 if(!s||!io||s->words[5]!=02651u||s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[4]!=012000u||s->words[6]!=(agc_word)(012000u|(s->words[3]>>8)))return CM_ENEMA_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&views.e);cm_native_gather_waitlist(s,io,&views.w);cm_native_gather_restart(s,io,&views.r);cm_native_gather_phase(s,io,&views.p);cm_native_gather_enema_io(s,io,&views.cpu);status=cm_enema_begin(&views.r,&views.e,&views.w,&views.p.records,&views.cpu);if(status==CM_ENEMA_OUT_OF_SCOPE)return status;
 cm_native_commit_executive(s,io,&views.e);cm_native_commit_waitlist(s,io,&views.w);cm_native_commit_restart(s,io,&views.r);cm_native_commit_enema_io(s,io,&views.cpu);s->words[5]=status==CM_ENEMA_DISPLAY?05360u:status==CM_ENEMA_BAD_PHASE?02746u:02365u;return status;
}
