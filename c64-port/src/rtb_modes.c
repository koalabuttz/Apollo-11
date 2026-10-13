/* SPDX-License-Identifier: MIT */
#include "rtb_modes.h"
enum rtb_status rtb_dpmode(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission)
{
    uint16_t *w=image->words;uint16_t fb;
    if((unsigned)mission>1u)return RTB_OUT_OF_SCOPE;
    fb=(uint16_t)(mission==RTB_LM?020000u:044000u);
    if(w[5]!=03716u||w[4]!=fb||w[3]>03400u||(w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||
       io->interrupt_enabled>1u||io->superbank>0160u||(io->superbank&017u))return RTB_OUT_OF_SCOPE;
    /* CAF ZERO; TS MPAC +2; TS MODE (A +0: no overflow skip); TCF DANZIG. */
    w[0]=0;w[0156]=0;w[0163]=0;
    w[5]=(uint16_t)(mission==RTB_LM?06061u:06030u);
    return RTB_OK;
}
