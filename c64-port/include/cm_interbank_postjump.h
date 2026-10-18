/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERBANK_POSTJUMP_H
#define CM_INTERBANK_POSTJUMP_H
/* POSTJUMP (04701) alone, as cm_interbank_routines runs it
 * (src/cm_interbank_complete.c), for the CM self-check idle loop's CHECKNJ:
 * the flight store packs it beside the pinned idle set (CO_LOCATE in
 * tools/flight_store_build.py), so an idle pass loads no job-slot module.
 * The program and the resolver keep cm_interbank_routines. io is a
 * cm_native_io. tests/test_cm_interbank_postjump.c checks it against
 * cm_interbank_routines' POSTJUMP on random states. */
#include "agc_transfer.h"
extern const agc_routine_table cm_interbank_postjump_hot_routines;
#endif
