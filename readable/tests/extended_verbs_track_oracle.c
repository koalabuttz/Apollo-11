/* SPDX-License-Identifier: MIT */
/* The port's frozen oracle rows for the tracking and state vector verbs'
 * basic code on the readable routines (make extended-verbs-cases): each
 * row is a case of the port's suite (c64-port/tests/lanec_ev_track_cases.c:
 * every entry of both missions, the flag words, EXTVBACT and E-bank as the
 * fill leaves them, and a VAC area's address in FIXLOC) with the hash of
 * the original program's complete result in Virtual AGC
 * (c64-port/tests/lanec_ev_track_c64.h, tools/lanec/freeze.py: the rows
 * that reach every branch of the port's module). The suite runs
 * lanec_ev_track_dispatch, which this program links from the readable
 * objects (extended_verbs_missions.c); it takes its list of entries from
 * the faithful module. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_track_c64.h"

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
        lanec_suite_ev_track.make(lanec_frozen[i].n, &s, &io, &meta);
        status = lanec_suite_ev_track.run(&s, &io, meta.mission);
        if (lanec_hash(&s, &io, status) != lanec_frozen[i].hash) {
            printf("FAIL: case %u (%s, Z %04o)\n", (unsigned)lanec_frozen[i].n,
                   meta.mission == LANEC_LM ? "LM" : "CM", s.words[5]);
            ++failures;
        }
    }
    printf("ev-track oracle: %u frozen rows, %u failures\n",
           (unsigned)(sizeof lanec_frozen / sizeof lanec_frozen[0]), failures);
    return failures != 0;
}
