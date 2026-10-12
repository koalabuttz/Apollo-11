/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_suites.h"
static const lanec_suite *const suites[] = { &lanec_suite_ev_front, &lanec_suite_pin_gaps, &lanec_suite_di_dsp, &lanec_suite_di_mark };
const lanec_suite *lanec_find_suite(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i)
        if (!strcmp(suites[i]->name, name)) return suites[i];
    return 0;
}
