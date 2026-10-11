/* SPDX-License-Identifier: MIT */
#ifndef SBAND_NAVFRAME_H
#define SBAND_NAVFRAME_H
#include "sband_bodyframe.h"
enum sband_navframe_status {SBAND_NAVFRAME_UNSUPPORTED,SBAND_NAVFRAME_OK,SBAND_NAVFRAME_ABORT_1302};
/* Actual returned-SMNB antenna vector store, LM body-axis mixing, and UNIT.
 * Five source VAC/PUSH bases, ordinary image/Q, coherent original returned
 * bank tuple, NEWJOB+0 and producer exclusion across all internal pairs.
 * Ends before actual pending PDVL or before source SQRTABRT, not an alarm
 * handler. No opcode decoder or added scheduling check. */
enum sband_navframe_status sband_navframe(apollo_overlay_state *,p66_bias_io *,enum apollo_fixed_mission);
#endif
