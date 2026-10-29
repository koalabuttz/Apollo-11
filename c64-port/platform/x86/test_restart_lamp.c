/* SPDX-License-Identifier: MIT */
/* Checks the x86 program's RESTART lamp (fd_state.restart_light) through
 * the dispatcher's own calls, one path at a time: power-up and a GOJAM
 * light it, RSET (fd_key 022) and channel 11 bit 10 (fd_lamps) each put it
 * out, and other keys and other channel 11 bits leave it alone. In flight
 * RSET always takes both clear paths in the same centisecond (ERROR writes
 * channel 11 bit 10), so the lockstep rows cannot tell them apart; this
 * can. tools/x86_flight_check.py links it with each mission's dispatcher. */
#include <stdio.h>
#include "flight_dispatch.h"

static int failures;
static fd_state st;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

int main(void)
{
    agc_word code;
    fd_init(&st, FD_INTERP_COMPILED, 037777u, 077777u);
    check(st.restart_light == 1u, "power-up lights the lamp");
    for (code = 1u; code < 040u; ++code)
        if (code != 022u) fd_key(&st, code);
    check(st.restart_light == 1u, "keys other than RSET leave it lit");
    st.hw.channel11 = 077777u & ~01000u;
    fd_lamps(&st);
    check(st.restart_light == 1u, "channel 11 without bit 10 leaves it lit");
    fd_key(&st, 022u);
    check(st.restart_light == 0u, "RSET puts it out with channel 11 bit 10 clear");
    fd_gojam(&st);
    check(st.restart_light == 1u && st.hw.channel11 == 0u,
          "a GOJAM lights it and clears channel 11");
    st.hw.channel11 = 01000u;
    fd_lamps(&st);
    check(st.restart_light == 0u, "channel 11 bit 10 puts it out with no key");
    fd_gojam(&st);
    st.hw.channel11 = 0u;
    fd_lamps(&st);
    check(st.restart_light == 1u, "after a GOJAM, channel 11 written with 0 leaves it lit");
    if (failures) return 1;
    printf("RESTART lamp: all checks pass\n");
    return 0;
}
