/* SPDX-License-Identifier: MIT */
/* pw-cm-charin-number: Comanche CHARIN (40,2000) through NUM to ENDOFJOB
 * (src/cm_charin_number.c), over the generator's whole 8192-number domain, plus
 * DECTOBIN's fifth-digit carry (lanec_pw_fifth_char). */
#include "lanec_pw_f.h"
#include "cm_charin_number_cases.h"

static uint16_t wrote;
LANEC_PW_F_CM_RUN(charin_number_run, cm_charin_number(s, &c, &wrote) == EXEC_OK)
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    cm_native_io c;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, CM_CHARIN_NUMBER_CASES);
    lanec_fill(s, io, n);
    cm_charin_number_input(s, &c, &wrote, k < CM_CHARIN_NUMBER_CASES ? k : (uint16_t)(k - CM_CHARIN_NUMBER_CASES));
    if (k >= CM_CHARIN_NUMBER_CASES) lanec_pw_fifth_char(s, k - CM_CHARIN_NUMBER_CASES);
    lanec_pw_f_from_cm(io, &c);
    lanec_pw_settle(s);
    m->mission = LANEC_CM;
    lanec_pw_exit_stop(m, s, io, charin_number_run);
}
const lanec_suite lanec_suite_pw_cm_charin_number = { "pw-cm-charin-number", CASES + LANEC_PW_FIFTH_CHAR, make, charin_number_run };
