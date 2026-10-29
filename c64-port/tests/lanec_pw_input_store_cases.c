/* SPDX-License-Identifier: MIT */
/* pw-input-store: PUTCOM/PUTNORM/PUTDPCOM/PUTCOM2 data stores
 * (src/dsky_input_store.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_input_store.h"
#include "dsky_input_store_cases.h"

#define CASES 1024u
/* past the domain: the module's own states with the store in low erasable
 * (the CDU counters of noun 20, the clock of noun 36) */
static void input(apollo_overlay_state *s, dsky_lamp_io *p, uint16_t k)
{
    if (k < 8192u) { dsky_input_store_input(s, p, k); return; }
    dsky_input_store_input(s, p, (uint16_t)((k - 8192u) * 127u % 8192u));
    lanec_pw_low_noun(s, k - 8192u);
}
LANEC_PW_MAKE(make, dsky_lamp_io, input, lanec_pw_lamp_out, lanec_pw_input_store_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_input_store = { "pw-input-store", CASES + LANEC_PW_LOW_NOUN, make, lanec_pw_input_store_run };
