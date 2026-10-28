/* SPDX-License-Identifier: MIT */
/* Checks platform/x86/hand_controller.h, the LM's hand controller: the
 * detent bit, the counters only with channel 13 bits 8 and 9 on, the
 * negative counts, and the bad inputs (make x86-flight-check builds and
 * runs it). */
#include <stdio.h>
#include <string.h>
#include "hand_controller.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

int main(void)
{
    static agc_word w[2048];
    hc_config c;
    agc_word ch31 = 077777u;

    check(hc_parse(&c, 0) && c.n == 0, "no FLIGHT_RHC");
    hc_step(&c, 100, w, 0600u, &ch31);
    check(ch31 == 077777u && w[HC_P] == 0, "no stick: nothing changes");

    check(hc_parse(&c, "2500-3100:20,0,0;3600-4200:0,-12,15") && c.n == 2, "two spans");
    hc_step(&c, 2400, w, 0600u, &ch31);
    check(ch31 == 077777u, "in detent before the first span");
    hc_step(&c, 2500, w, 0200u, &ch31);
    check(ch31 == 037777u && w[HC_P] == 0, "out of detent; counters wait for bits 8 and 9");
    hc_step(&c, 2600, w, 0600u, &ch31);
    check(w[HC_P] == 20u && w[HC_Q] == 0 && w[HC_R] == 0, "pitch +20");
    hc_step(&c, 3100, w, 0600u, &ch31);
    check(ch31 == 077777u && w[HC_P] == 20u, "back in detent at TO; counters left to the DAP");
    hc_step(&c, 3700, w, 0600u, &ch31);
    check(w[HC_Q] == 077763u && w[HC_R] == 15u, "yaw -12 in ones' complement, roll +15");

    check(!hc_parse(&c, "100-50:1,2,3"), "TO before FROM");
    check(!hc_parse(&c, "100-200:43,0,0"), "a count over 42");
    check(!hc_parse(&c, "300-400:1,0,0;100-200:1,0,0"), "out of time order");
    check(!hc_parse(&c, "100-200:1,0"), "two counts");

    if (failures) return 1;
    printf("hand controller: all checks pass\n");
    return 0;
}
