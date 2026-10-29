/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCKTASK_H
#define CM_CLOCKTASK_H
#include "cm_native_image.h"
enum cm_clocktask_exit { CM_CLOCK_TASKOVER=0,CM_CLOCK_WAITLIST=1,CM_CLOCK_NOVAC=2,CM_CLOCK_FINDVAC=3,CM_CLOCK_INVALID=4 };
/* Actual Comanche CLOKTASK program groups: phase0 task entry through WAITLIST
 * (or disabled-timer phase cleanup/TASKOVER), phase1 continuation after WAITLIST
 * through selected display-job admission or TASKOVER, phase2 SETTB6 through
 * TASKOVER. Each explicit native phase returns before the resident executes a
 * service/loads another module; no C frame survives a source group boundary.
 * Whole image + separate SUPERBNK/IRQ metadata are authoritative. */
enum cm_clocktask_exit cm_clocktask_phase(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
