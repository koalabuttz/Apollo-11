/* SPDX-License-Identifier: MIT */
/* pw-input-time: HMSIN with RND/TST, MPACTST and SIZETST
 * (src/dsky_input_time.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_input_time.h"
#include "dsky_input_time_cases.h"

#define CASES 1024u
/* past the domain: Comanche SIZETST's alarm (TC ALMCYCLE, 42,3421) */
static const uint16_t extra[] = { 4640, 6676 };
static void input(apollo_overlay_state *s, dsky_lamp_io *p, uint16_t k)
{
    dsky_input_time_input(s, p, k < 8192u ? k : extra[k - 8192u]);
}
LANEC_PW_MAKE(make, dsky_lamp_io, input, lanec_pw_lamp_out, lanec_pw_input_time_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_input_time = { "pw-input-time", CASES + sizeof extra / sizeof extra[0], make, lanec_pw_input_time_run };
