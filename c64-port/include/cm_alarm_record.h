/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_RECORD_H
#define CM_ALARM_RECORD_H
#include "cm_native_image.h"
enum cm_alarm_record_entry {CM_RECORD_ALARM,CM_RECORD_VARALARM,CM_RECORD_CURTAINS,CM_RECORD_BORTENT};
/* Actual non-abortive alarm recording body and genuine return. Inline alarm
 * data for ALARM is caller-qualified passive source data, not decoded code.
 * VARALARM/BORTENT require a nonoverflow raw A alarm code. BORTENT preserves
 * its earlier ALMCADR producer payload. MULTFAIL overflow follows its actual
 * skipped branch to POODOO5721 with IRQ inhibited, rather than claiming a
 * caller return. Caller exclusion; selected abort body remains later work. */
enum exec_status cm_alarm_record(apollo_overlay_state *,cm_native_io *,enum cm_alarm_record_entry,agc_word inline_alarm);
#endif
