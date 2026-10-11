/* SPDX-License-Identifier: MIT */
#ifndef SBAND_FLOW_H
#define SBAND_FLOW_H
#include "sband_control.h"
enum sband_flow_entry {SBAND_FLOW_TIME_FRONT,SBAND_FLOW_AFTER_TIME,SBAND_FLOW_TIME_PIPELINE,SBAND_FLOW_AFTER_CDU,SBAND_FLOW_CM_TIME_TRIG};
/* Compiled original antenna LOADTIME/SETPD/RTB/STCALL/CALL caller stages.
 * TIME_FRONT ends at actual banked LOADTIME entry; AFTER_TIME reaches LM
 * LEMCONIC or CM CDUTRIG at original NEWOPS/LOC. TIME_PIPELINE includes native
 * LOADTIME. CM_TIME_TRIG additionally executes native CDUTRIG, real reentry
 * and RVQ, then AFTER_CDU's STCALL to CSMCONIC. Orbital bodies are genuine
 * external continuations. No interpreted dispatcher or scheduling poll.
 * Original source bank/PC/control guards, valid five VAC bases, NEWJOB+0,
 * ordinary RAM/raw16 A/L/Q and SUPER selecting physical42 required. Image
 * remains authoritative; producers excluded throughout all composed phases.
 * Rejection of unsupported entry contexts preserves image/external latches. */
bool sband_flow(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission,
                enum sband_flow_entry);
#endif
