/* SPDX-License-Identifier: MIT */
#include "rtb_pulse_return.h"
enum rtb_status rtb_pulse_return(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission)
{
 uint16_t *w=m->words;agc_word fb;
 if((unsigned)mission>1u)return RTB_OUT_OF_SCOPE;
 fb=(agc_word)(mission==RTB_LM?020000u:044000u);
 if(w[5]!=(mission==RTB_LM?03711u:03657u)||w[4]!=fb||w[3]>03400u||(w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||io->interrupt_enabled>1u||(io->superbank&~0160u))return RTB_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[5]=(agc_word)(mission==RTB_LM?06061u:06030u);return RTB_OK;
}
