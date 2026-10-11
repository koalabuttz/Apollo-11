/* SPDX-License-Identifier: MIT */
#ifndef CM_ENEMA_H
#define CM_ENEMA_H
#include "cm_restart.h"
typedef struct {
 agc_word main_keys,navigation_keys;
 agc_register a,l;
 agc_word q,bbank,superbank,interrupt_enabled;
 agc_word waitexit,waitbank; /* Physical aliases0061/0062, preserved unless TVC WAITLIST. */
} cm_enema_io;
enum cm_enema_result { CM_ENEMA_DISPLAY=0,CM_ENEMA_FRESH_START=1,CM_ENEMA_OUT_OF_SCOPE=2,CM_ENEMA_BAD_PHASE=01107 };
/* CM ENEMA including LIGHTSET, STARTSB2, integration-bit clearing, conditional
 * TVCEXEC WAITLIST and complete phase check; stop BEFORE MMDSPLAY/PTBAD/DOFSTART.
 * Fresh route performs STARTSUB cleanup before reaching DOFSTART. Counter and
 * source publication stopped; distinct canonical-image views, normal15-bit
 * words and physical registers required. Display/flight restart execution is
 * a subsequent explicit continuation boundary, never a resumed C stack. */
enum cm_enema_result cm_enema_begin(cm_restart_state *,cm_executive *,cm_waitlist *,const cm_restart_phases *,cm_enema_io *);
#endif
