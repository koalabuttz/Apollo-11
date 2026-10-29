/* SPDX-License-Identifier: MIT */
#ifndef CM_ENTRY_TASKS_H
#define CM_ENTRY_TASKS_H
#include "cm_native_image.h"
#include "cm_waitlist_native.h"
/* Original SETJTAG at 15,3207 through actual TASKOVER entry. Saves -TIME1 in
 * E6 TUSED, sets JETAG=1, clears restart group1 and source RELINT enables IRQ.
 * Physical source bank15/E6 and valid external latches are caller obligations.
 * No suspended C stack, duplicate flight image or assembly decoder is used. */
enum cm_wl_status cm_setjtag_image(apollo_overlay_state *,cm_native_io *);
typedef struct { apollo_overlay_state *image;cm_native_io *io; } cm_entry_task_binding;
/* Resident WAITLIST mapping callback for actual source SETJTAG 2CADR. Dispatch
 * must publish its source bank to the canonical image before invoking it.
 * The callback returns before the existing trampoline executes TASKOVER. */
enum cm_wl_status cm_setjtag_callback(cm_wl_native *,void *);
#endif
