/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_H
#define CM_WAITLIST_H
#include "executive.h"
#define CM_WL_TASK_COUNT 9
#define CM_WL_INTERVAL_COUNT 8
#define CM_WL_MAX_DELAY UINT16_C(16250)
#define CM_WL_END_LOCATION UINT16_C(05306)
#define CM_WL_END_BANK UINT16_C(04063)
#define CM_WL_DEFAULT_INTERVAL UINT16_C(057777)
enum cm_wl_status { CM_WL_OK=0, CM_WL_OUT_OF_SCOPE=1, CM_WL_ALARM_PENDING=2, CM_WL_IDLE=3,
                 CM_WL_UNMAPPED=4, CM_WL_REENTER=5, CM_WL_SOURCE_OVERFLOW=6,
                 CM_WL_CALLBACK_FAULT=7, CM_WL_FULL=01203, CM_WL_BAD_DELAY=01204 };
typedef struct {
    agc_word time3, interval[CM_WL_INTERVAL_COUNT];
    exec_address task[CM_WL_TASK_COUNT];
    agc_word ruptagn, alarm;
} cm_waitlist;
/* Resource markers from fresh start; not the SVCT3 housekeeping task body.
 * Raw primitives require caller exclusion. Words retain both signed zeros. */
void cm_wl_init(cm_waitlist *state);
/* WAITLIST insertion through LVWTLIST or the 1203/1204 alarm entry boundary.
 * Positive delays above the documented maximum are rejected before mutation.
 * Full-list insertion may already have displaced its last task: no rollback. */
enum cm_wl_status cm_wl_schedule(cm_waitlist *state, agc_word delay, exec_address task);
/* Controlled centisecond counter increments, matching TIME3 PINC's +overflow
 * reset to +0. Returns 1 if any overflow occurred, 0 otherwise, -1 on bad state.
 * The platform/runtime owns the pending-interrupt latch. */
int cm_wl_advance(cm_waitlist *state, uint16_t centiseconds);
/* T3RUPT2 list/time update and basic task transfer. Tasks finish via taskover.
 * The dummy SVCT3 address is returned as metadata; its body is not translated. */
enum cm_wl_status cm_wl_begin(cm_waitlist *state, exec_address *next);
enum cm_wl_status cm_wl_taskover(cm_waitlist *state, exec_address *next);
#endif
