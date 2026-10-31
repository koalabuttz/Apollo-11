/* SPDX-License-Identifier: MIT */
/* The port's frozen oracle rows for the clock update (verb 55) and the
 * LM's ATTCK2 on the readable routines (make extended-verbs-cases): each
 * row is a case of the port's suite (c64-port/tests/lanec_ev_time_cases.c:
 * every entry of both missions, the verb in MPAC, the clock and the
 * increment, THETAD and each axis index at ATTCK2 +1) with the hash of the
 * original program's complete result in Virtual AGC
 * (c64-port/tests/lanec_ev_time_c64.h, tools/lanec/freeze.py: the rows
 * that reach every branch of the port's module). The suite runs
 * lanec_ev_time_dispatch, which this program links from the readable
 * objects (extended_verbs_missions.c); it takes its list of entries from
 * the faithful module. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_time_c64.h"

int main(void)
{
    unsigned i, failures = 0;

    for (i = 0; i < sizeof lanec_frozen / sizeof lanec_frozen[0]; ++i) {
        apollo_overlay_state s;
        lanec_io io;
        lanec_meta meta;
        enum lanec_status status;

        memset(&s, 0, sizeof s);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_time.make(lanec_frozen[i].n, &s, &io, &meta);
        status = lanec_suite_ev_time.run(&s, &io, meta.mission);
        if (lanec_hash(&s, &io, status) != lanec_frozen[i].hash) {
            printf("FAIL: case %u (%s, Z %04o)\n", (unsigned)lanec_frozen[i].n,
                   meta.mission == LANEC_LM ? "LM" : "CM", s.words[5]);
            ++failures;
        }
    }
    printf("ev-time oracle: %u frozen rows, %u failures\n",
           (unsigned)(sizeof lanec_frozen / sizeof lanec_frozen[0]), failures);
    return failures != 0;
}
