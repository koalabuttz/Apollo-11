/* SPDX-License-Identifier: MIT */
/* pw-charin-number: Luminary CHARIN (40,2077) for a digit key through NUM/DECTOBIN/MORNUM,
 * ENDNUM/DECEND and +DECSGN to ENDOFJOB (src/dsky_charin_number.c), the LM
 * side of pw-cm-charin-number; 8192-number domain plus the fifth-digit carry. */
#include "lanec_pw_g.h"
#include "dsky_charin_number.h"
#include "dsky_charin_number_cases.h"

LANEC_PW_G_CHARIN_RUN(charin_number_run, dsky_charin_number(s, &p))
#define CASES 2048u
/* past the generator's domain: DECTOBIN's fifth-digit carry (lanec_pw_fifth_char) */
static void prepare(apollo_overlay_state *s, uint16_t k)
{
    if (k < DSKY_CHARIN_NUMBER_CASE_COUNT) { dsky_charin_number_prepare(s, k); return; }
    dsky_charin_number_prepare(s, (uint16_t)(k - DSKY_CHARIN_NUMBER_CASE_COUNT));
    lanec_pw_fifth_char(s, k - DSKY_CHARIN_NUMBER_CASE_COUNT);
}
LANEC_PW_G_MAKE(make, prepare, DSKY_CHARIN_NUMBER_CASE_COUNT, CASES, charin_number_run, dsky_charin_number_channel11(k))
const lanec_suite lanec_suite_pw_charin_number = { "pw-charin-number", CASES + LANEC_PW_FIFTH_CHAR, make, charin_number_run };
