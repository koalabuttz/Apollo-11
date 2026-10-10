/* SPDX-License-Identifier: MIT */
#ifndef WAITLIST_H
#define WAITLIST_H
#include "executive.h"
#define WL_TASK_COUNT 9
#define WL_INTERVAL_COUNT 8
#define WL_MAX_DELAY UINT16_C(16250)
#define WL_END_LOCATION UINT16_C(05240)
#define WL_END_BANK UINT16_C(04063)
#define WL_DEFAULT_INTERVAL UINT16_C(057777)
enum wl_status { WL_OK=0, WL_OUT_OF_SCOPE=1, WL_ALARM_PENDING=2, WL_IDLE=3,
                 WL_UNMAPPED=4, WL_REENTER=5, WL_SOURCE_OVERFLOW=6,
                 WL_CALLBACK_FAULT=7, WL_FULL=01203, WL_BAD_DELAY=01204 };
typedef struct {
    agc_word time3, interval[WL_INTERVAL_COUNT];
    exec_address task[WL_TASK_COUNT];
    agc_word ruptagn, alarm;
} waitlist;
/* Resource markers from fresh start; not the SVCT3 housekeeping task body.
 * Raw primitives require caller exclusion. Words retain both signed zeros. */
void wl_init(waitlist *state);
/* WAITLIST insertion through LVWTLIST or the 1203/1204 alarm entry boundary.
 * Positive delays above the documented maximum are rejected before mutation.
 * Full-list insertion may already have displaced its last task: no rollback. */
enum wl_status wl_schedule(waitlist *state, agc_word delay, exec_address task);
/* Controlled centisecond counter increments, matching TIME3 PINC's +overflow
 * reset to +0. Returns 1 if any overflow occurred, 0 otherwise, -1 on bad state.
 * The platform/runtime owns the pending-interrupt latch. */
int wl_advance(waitlist *state, uint16_t centiseconds);
/* T3RUPT2 list/time update and basic task transfer. Tasks finish via taskover.
 * The dummy SVCT3 address is returned as metadata; its body is not translated. */
enum wl_status wl_begin(waitlist *state, exec_address *next);
enum wl_status wl_taskover(waitlist *state, exec_address *next);
#endif
