/* SPDX-License-Identifier: MIT */
#ifndef CM_DOWNTELEMETRY_H
#define CM_DOWNTELEMETRY_H
#include "agc_transfer.h"
#include "cm_native_image.h"
/* Comanche055 DOWN-TELEMETRY_PROGRAM as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). DODOWNTM is entered
 * from the DOWNRUPT lead-in (TS BANKRUPT with the interrupted BB in A, FB
 * 05) and ends in TC DNTMGOTO: the erasable point DNTMGOTO executes the word
 * stored there as TC, accepting only the words this program and its
 * producers store (DNPHASE1, DNPHASE2, DNDUMPI, DNDUMP, DNDUMP1 and the return
 * of DNDUMPI's TC SENDID). Each of those ends at TCF RESUME after WRITE DNTM1,
 * DNTM2. Downlists and constants are compiled from the source listing
 * (src/cm_downtelemetry_data.inc).
 *
 * io is a cm_dntm_io: the mission's cm_native_io (channels 11-14 and the
 * channel-13 traps) plus input channels 30-33 (read by DNCHAN entries) and
 * output channels 34/35 (DNTM1/DNTM2). */
typedef struct { cm_native_io io; agc_word channel30,channel31,channel32,channel33,channel34,channel35; } cm_dntm_io;
#define CM_DNTM_DODOWNTM 03521u   /* bank 05 */
#define CM_DNTM_DNPHASE1 03530u
#define CM_DNTM_DNPHASE2 03536u
#define CM_DNTM_DNDUMPI 03722u
#define CM_DNTM_DNDUMPI3 03725u
#define CM_DNTM_DNDUMP 03735u
#define CM_DNTM_DNDUMP1 03750u
#define CM_DNTM_RESUME 05333u     /* WAITLIST */
#define CM_DNTM_DNTMGOTO 0335u    /* erasable */
#define CM_DNTM_DNLSTCOD 0332u
#define CM_DNTM_DUMPCNT 0333u
#define CM_DNTM_ROUTINES 8u
extern const agc_routine_table cm_downtelemetry_routines;
#endif
