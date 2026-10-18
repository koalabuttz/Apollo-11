/* SPDX-License-Identifier: MIT */
/* pw-load-store: the load callers, PUTCOM returns/stores and noun-7 bit
 * controls (src/dsky_load_store.c). Generator numbers 0-8191 are LM,
 * 8192-16383 CM. In each half the first 4096 are the module's own cases;
 * the second 4096 repeat them with the store in low erasable (E0-E2 and
 * unswitched 0077-01077, as the N70/N79 loads of LM P57 and the CM V25
 * loads store), where the store bound 0100 is at its edge. */
#include "lanec_pw_c.h"
#include "dsky_load_store.h"
#include "dsky_load_store_cases.h"

#define CASES 1024u

static void input(apollo_overlay_state *s, dsky_lamp_io *io, uint16_t n)
{
    static const uint16_t banks[3] = {0, 0400u, 01000u};
    static const uint16_t nounadd[8] = {01500u, 01735u, 01477u, 0763u, 01432u, 077u, 01077u, 024u};
    uint16_t j = (uint16_t)(n % 4096u), *w = s->words;
    dsky_load_store_input(s, io, (uint16_t)(n / 8192u * 4096u + j));
    if (n % 8192u < 4096u) return;
    w[3] = banks[(j / 16u) % 3u];
    w[6] = (uint16_t)(062000u | (w[3] >> 8));
    w[0145] = nounadd[(j / 128u) % 8u];
}

LANEC_PW_MAKE(make, dsky_lamp_io, input, lanec_pw_lamp_out, lanec_pw_load_store_run, 16384u, CASES)
const lanec_suite lanec_suite_pw_load_store = {"pw-load-store", CASES, make, lanec_pw_load_store_run};
