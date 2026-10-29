/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_ROTATION_H
#define GUIDANCE_ROTATION_H
#include "guidance_trig.h"
typedef struct {agc_word dex2,rtnsaver;} p66_rotation_scratch;
/* Original basic AX*SR*T: vector arrives and returns in norm.VBUF; stable
 * SINCDU/COSCDU arrays are trig.sin/cos in Y/Z/X order at half amplitude.
 * direction is +3 or -3 (077774), NB->SM or SM->NB respectively. All MODE
 * words are retained: this basic routine does not consult MODE. Retain full
 * MPAC/TERM1TMP, BUF, MPTEMP, DEXDEX/TEM2, DEX1/TEM3, DEX2 and A/L debris.
 * return_word is a 12-bit caller Q saved at RTNSAVER0145; stop before executing
 * that saved return. Invalid direction/return bits reject all state unchanged.
 * Distinct owned ordinary-word objects and producer exclusion are obligations.
 * Historical geometric guarantee is vector magnitude below unity; raw values
 * outside that guarantee still follow the original overflow arithmetic.
 * No trig generation, interpreter, native dispatch, heap or floating point. */
uint8_t p66_axis_rotate(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,
    p66_divide_scratch *,p66_norm_scratch *,p66_trig_scratch *,p66_rotation_scratch *,
    agc_word direction,agc_word return_word);
#endif
