/* SPDX-License-Identifier: MIT */
#ifndef CM_MYSUBS_ROUTINES_H
#define CM_MYSUBS_ROUTINES_H
#include "agc_transfer.h"
/* The flight store's table for Comanche MYSUBS (docs/FLIGHT-STORE.md):
 * SMALLMP (21,2042) and DPADD (21,2054), each the certified body
 * (src/cm_mysubs.c) and its TC Q, entered at its original address with the
 * RCS DAP's banks (FB 21, EB 6) and returning to the caller's Q. */
extern const agc_routine_table cm_mysubs_routines;
#endif
