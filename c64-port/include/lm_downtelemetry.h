/* SPDX-License-Identifier: MIT */
#ifndef LM_DOWNTELEMETRY_H
#define LM_DOWNTELEMETRY_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 DOWN-TELEMETRY_PROGRAM as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). DODOWNTM is entered
 * from the DOWNRUPT lead-in (TS BANKRUPT with the interrupted BB in A, FB
 * 05), sets the word-order bit through WOTEST when it is clear, and ends in
 * TC DNTMGOTO: the erasable point DNTMGOTO executes the word
 * stored there as TC, accepting only the words this program and its
 * producers store (DNPHASE1, DNPHASE2, DNDUMPI, DNDUMP, DNDUMP1 and the return
 * of DNDUMPI's TC SENDID). Each of those ends at TCF RESUME after WRITE DNTM1,
 * DNTM2. Downlists and constants are compiled from the source listing
 * (src/lm_downtelemetry_data.inc).
 *
 * io is a lm_dntm_io: the mission's p66_bias_io, channels 11-14 with the
 * channel-13 traps, input channels 30-33 (read by DNCHAN entries) and output
 * channels 34/35 (DNTM1/DNTM2). */
typedef struct { p66_bias_io control; agc_word channel11,channel12,channel13,channel14,channel30,channel31,channel32,channel33,channel34,channel35;
 uint16_t trap31a,trap31b,trap32; } lm_dntm_io;
#define LM_DNTM_DODOWNTM 03506u   /* bank 05 */
#define LM_DNTM_DNPHASE1 03515u
#define LM_DNTM_DNPHASE2 03523u
#define LM_DNTM_DNDUMPI 03707u
#define LM_DNTM_DNDUMPI3 03712u
#define LM_DNTM_DNDUMP 03722u
#define LM_DNTM_DNDUMP1 03735u
#define LM_DNTM_RESUME 05270u     /* WAITLIST */
#define LM_DNTM_DNTMGOTO 0335u    /* erasable */
#define LM_DNTM_DNLSTCOD 0332u
#define LM_DNTM_DUMPCNT 0333u
#define LM_DNTM_ROUTINES 8u
extern const agc_routine_table lm_downtelemetry_routines;
#endif
