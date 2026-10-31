/* SPDX-License-Identifier: MIT */
/* The port's frozen oracle rows for the LM's interpretive words of the
 * tracking verbs on the readable interpreter (make extended-verbs-cases):
 * each row is a case of the port's suite (c64-port/tests/
 * lanec_ev_lmi_cases.c: every entry, a VAC area or NOVAC's in FIXLOC, a
 * job change pending in every fourth, the lines of sight UNIT alarms on or
 * takes its small-vector path for) with the hash of the original
 * program's complete result in Virtual AGC (c64-port/tests/
 * lanec_ev_lmi_c64.h, tools/lanec/freeze.py: the rows that reach every
 * branch of the port's module). The suite runs lanec_ev_lmi_dispatch,
 * which this program links from the readable objects
 * (extended_verbs_missions.c); it takes its list of entries from the
 * faithful module. The readable interpreter reads the program's constants
 * (UNITZ, DPPOSMAX, ...) from the rope, as the AGC's does: the ropes are
 * named on the command line. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_lmi_c64.h"
#include "rope_image.h"

int main(int argc, char **argv)
{
    unsigned i, failures = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < sizeof lanec_frozen / sizeof lanec_frozen[0]; ++i) {
        apollo_overlay_state s;
        lanec_io io;
        lanec_meta meta;
        enum lanec_status status;
        unsigned loc;

        memset(&s, 0, sizeof s);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_lmi.make(lanec_frozen[i].n, &s, &io, &meta);
        loc = s.words[0164];
        status = lanec_suite_ev_lmi.run(&s, &io, meta.mission);
        if (lanec_hash(&s, &io, status) != lanec_frozen[i].hash) {
            printf("FAIL: case %u (LOC %04o)\n", (unsigned)lanec_frozen[i].n, loc);
            ++failures;
        }
    }
    printf("ev-lmi oracle: %u frozen rows, %u failures\n",
           (unsigned)(sizeof lanec_frozen / sizeof lanec_frozen[0]), failures);
    return failures != 0;
}
