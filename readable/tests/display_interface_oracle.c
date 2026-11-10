/* SPDX-License-Identifier: MIT */
/* The port's frozen oracle rows for DISPLAY_INTERFACE_ROUTINES on the
 * readable routines (make display-interface-cases): each row is a case of
 * one of the port's six suites (c64-port/tests/lanec_di_<part>_cases.c:
 * every entry of both missions, A with and without overflow, the display
 * kinds, the flag words' busy and waiting bits, the returns the R displays
 * save, the job register sets) with the hash of the original program's
 * complete result in Virtual AGC (c64-port/tests/lanec_di_<part>_c64.h,
 * tools/lanec/freeze.py: the rows that reach every branch of the port's
 * module). The suites run lanec_di_<part>_dispatch, which this program
 * links from the readable objects (display_interface_missions.c). argv:
 * the LM and CM ropes, which the readable code reads (tests/with_ropes.c). */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"

#define lanec_frozen frozen_dsp
#include "lanec_di_dsp_c64.h"
#undef lanec_frozen
#define lanec_frozen frozen_mark
#include "lanec_di_mark_c64.h"
#undef lanec_frozen
#define lanec_frozen frozen_play
#include "lanec_di_play_c64.h"
#undef lanec_frozen
#define lanec_frozen frozen_sleep
#include "lanec_di_sleep_c64.h"
#undef lanec_frozen
#define lanec_frozen frozen_idle
#include "lanec_di_idle_c64.h"
#undef lanec_frozen
#define lanec_frozen frozen_nvdsp
#include "lanec_di_nvdsp_c64.h"
#undef lanec_frozen

struct row {
    uint32_t n, hash;
};

#define PART(suite, rows) {&suite, (const struct row *)rows, sizeof rows / sizeof rows[0]}
static const struct {
    const lanec_suite *suite;
    const struct row *rows;
    size_t count;
} parts[] = {
    PART(lanec_suite_di_dsp, frozen_dsp),   PART(lanec_suite_di_mark, frozen_mark),
    PART(lanec_suite_di_play, frozen_play), PART(lanec_suite_di_sleep, frozen_sleep),
    PART(lanec_suite_di_idle, frozen_idle), PART(lanec_suite_di_nvdsp, frozen_nvdsp),
};

int port_main(void)
{
    unsigned k, i, rows = 0, failures = 0;

    for (k = 0; k < sizeof parts / sizeof parts[0]; ++k) {
        for (i = 0; i < parts[k].count; ++i, ++rows) {
            apollo_overlay_state s;
            lanec_io io;
            lanec_meta meta;
            enum lanec_status status;

            memset(&s, 0, sizeof s);
            memset(&io, 0, sizeof io);
            memset(&meta, 0, sizeof meta);
            parts[k].suite->make(parts[k].rows[i].n, &s, &io, &meta);
            status = parts[k].suite->run(&s, &io, meta.mission);
            if (lanec_hash(&s, &io, status) != parts[k].rows[i].hash) {
                printf("FAIL: %s case %u (%s)\n", parts[k].suite->name,
                       (unsigned)parts[k].rows[i].n, meta.mission == LANEC_LM ? "LM" : "CM");
                ++failures;
            }
        }
    }
    printf("di oracle: %u frozen rows, %u failures\n", rows, failures);
    return failures != 0;
}
