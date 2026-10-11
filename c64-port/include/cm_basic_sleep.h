/* SPDX-License-Identifier: MIT */
#ifndef CM_BASIC_SLEEP_H
#define CM_BASIC_SLEEP_H
#include "cm_native_image.h"
#include "cm_dispatch.h"
/* Actual JOBSLEEP05175 through the selected basic DTCB, interpreted INTRSM
 * before NEWOPS06037, or idle ADVAN03231. Retains the dormant core and VAC.
 * Caller RAM words are ordinary15-bit values; A is an ordinary sign-extended
 * wake CADR, L/Q may retain raw16-bit patterns. Current priority is positive;
 * candidates are positive/dormant/-0 (source CCSHOLE +0 is excluded).
 * Selected saved basic LOC is1..07777, or a negative interpreted LOC which
 * advances to1..07777, with a legal saved BBCON. External SUPER/IRQ/channel11
 * are ordinary legal device values and no Executive alarm is pending.
 * Caller owns exclusion and distinct nonnull image/io/transfer objects.
 * Unsupported inputs preserve all three objects. No selected job body is
 * executed here: transfer reports the actual source-selected continuation. */
enum exec_status cm_basic_sleep(apollo_overlay_state *,cm_native_io *,exec_transfer *);
#endif
