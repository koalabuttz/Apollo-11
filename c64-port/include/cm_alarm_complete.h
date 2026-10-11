/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM_COMPLETE_H
#define CM_ALARM_COMPLETE_H
#include "cm_alarm_image.h"
/* Additive exact source entry wrapper. A normal BORTENT return may itself
 * target address05721; true overflow is distinguished by the TS resultA.
 * Existing frozen primitive/source artifacts remain byte unchanged. */
enum exec_status cm_alarm_complete(apollo_overlay_state *,cm_native_io *,enum cm_alarm_record_entry,agc_word);
#endif
