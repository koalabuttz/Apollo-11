/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_suites.h"
static const lanec_suite *const suites[] = { &lanec_suite_ev_front, &lanec_suite_pin_gaps, &lanec_suite_di_dsp, &lanec_suite_di_mark, &lanec_suite_di_play, &lanec_suite_di_sleep, &lanec_suite_di_idle, &lanec_suite_di_nvdsp, &lanec_suite_ev_short, &lanec_suite_ev_align, &lanec_suite_ev_radar, &lanec_suite_ev_dap, &lanec_suite_ev_time, &lanec_suite_ev_track, &lanec_suite_ev_lmi, &lanec_suite_di_transfer, &lanec_suite_di_transfer_c64, &lanec_suite_di_refuse };
const lanec_suite *lanec_find_suite(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i)
        if (!strcmp(suites[i]->name, name)) return suites[i];
    return 0;
}
