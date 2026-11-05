/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055): the
 * basic routines an interpretive RTB runs ("return to basic": RTB names a
 * routine of these, the interpreter's SWCALL goes there, and the routine
 * ends back at DANZIG). Only LOADTIME (for R63), SGNAGREE (for the LM's
 * star data test, R54), CDULOGIC (for AOTMARK's sightings and the
 * geometry), 2V1STO2S (for the LM's radar angles), V1STO2S (for the
 * maneuver's CDU angles, KALCMANU_STEERING), TPMODE (for the LM's
 * measurement incorporation, LSR22.3), DPMODE (for R36),
 * NORMUNX1 (for the LM's R31), NORMUNIT (for the LM's landing guidance)
 * and 1STO2S, TRANSP1 and TRANSP2 (for the CM's sextant geometry) are here
 * so far; the rest of the file is the faithful port's. */
#ifndef READABLE_RTB_OP_CODES_H
#define READABLE_RTB_OP_CODES_H

#include "../agc/agc.h"

void loadtime(void);
void sgnagree(void);
void cdulogic(void);
void one_sto2s(void);
void v1sto2s(void);
void two_v1sto2s(void);
void tpmode(void);
void dpmode(void);
void normunx1(void);
void normunit(void);
#if !LUMINARY099
void transp1(void);
void transp2(void);
#endif

#endif
