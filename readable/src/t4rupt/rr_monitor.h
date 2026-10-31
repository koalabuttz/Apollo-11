/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 3 (the LM's only): the rendezvous radar checks and GPMATRIX
 * (T4RUPT_PROGRAM.agc, RRAUTCHK to the end of the file).
 *
 * T4JUMP comes to RRAUTCHK twice a second: it watches the radar's power
 * and auto mode (channel 33 bit 2) and starts its turn-on, watches its CDU
 * fail bit (channel 30 bit 7) for the TRACKER lamp, and checks its
 * gimbals are within limits (RRGIMON), starting a reposition if not. All
 * three end at NORRGMON, which is GPMATRIX: the matrix elements the DAP
 * uses between gimbal and pilot axes, from the gimbal angles (T4JUMP also
 * goes straight there, as DAPT4S). GPMATRIX ends at RESUME.
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). */
#ifndef READABLE_RR_MONITOR_H
#define READABLE_RR_MONITOR_H

#include "../agc/agc.h"

#if LUMINARY099
void rrautchk(void);
void rrautchk_after_waitlist(void); /* RRAUTCHK +024: TCF NORRGMON */
void trkflcdu(void);                /* also ALARM's return */
void rrgimon(void);                 /* also SETTRKF's return */
void rrgimon_out_of_limits(void);   /* RRLIMCHK's return: TCF MONREPOS */
void rrgimon_within_limits(void);   /* RRLIMCHK's skip: TCF NORRGMON */
void monrepos_after_waitlist(void); /* MONREPOS +011: TCF NORRGMON */
void gpmatrix(void);                /* DAPT4S, NORRGMON */
void gpmatrix_after_sin_mg(void);   /* GPMATRIX +2, SPSIN's return */
void gpmatrix_after_cos_mg(void);   /* GPMATRIX +5, SPCOS's return */
void gpmatrix_after_sin_og(void);   /* GPMATRIX +010, SPSIN's return */
void gpmatrix_after_cos_og(void);   /* GPMATRIX +017, SPCOS's return */
#endif

#endif
