/* SPDX-License-Identifier: MIT */
#include "rtb_pulse_return_cases.h"
#include "rtb_angles_cases.h"
void rtb_pulse_return_input(apollo_overlay_state *m,p66_bias_io *io,enum rtb_mission mission,uint32_t n)
{
 rtb_angles_input(m,io,mission,RTB_LOAD_TIME,n);m->words[5]=(agc_word)(mission==RTB_LM?03711u:03657u);m->words[2]=(agc_word)(n&077777u);
}
