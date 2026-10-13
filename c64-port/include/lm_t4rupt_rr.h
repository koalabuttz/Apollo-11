/* SPDX-License-Identifier: MIT */
#ifndef LM_T4RUPT_RR_H
#define LM_T4RUPT_RR_H
#include "agc_transfer.h"
/* Luminary099 T4RUPT_PROGRAM.agc after the IMU monitor (bank 06,3006-3155):
 * the rendezvous radar monitor RRAUTCHK, RRCDUCHK, TRKFLCDU, RRGIMON and
 * MONREPOS, and GPMATRIX (DAPT4S, also NORRGMON), as routines of the
 * control-transfer ABI (milestone lm-t4rupt-rr, docs/LM-T4RUPT-RR.md).
 *
 * Entries: RRAUTCHK and GPMATRIX (T4JUMP's targets) and the continuation
 * after each call that leaves the file (WAITLIST, ALARM, SETTRKF, RRLIMCHK,
 * SPSIN, SPCOS). Each runs to the next of those calls or to RESUME.
 * Channels 12, 30 and 33 are program->hardware's. */
#define LM_T4RUPT_RR_ENTRIES 12u
extern const agc_routine_table lm_t4rupt_rr_routines;
#endif
