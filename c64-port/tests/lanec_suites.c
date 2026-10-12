/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_suites.h"
static const lanec_suite *const suites[] = { &lanec_suite_ev_front, &lanec_suite_pin_gaps, &lanec_suite_di_dsp, &lanec_suite_di_mark, &lanec_suite_di_play, &lanec_suite_di_sleep, &lanec_suite_di_idle, &lanec_suite_di_nvdsp, &lanec_suite_ev_short, &lanec_suite_ev_align, &lanec_suite_ev_radar, &lanec_suite_ev_dap, &lanec_suite_ev_time, &lanec_suite_ev_track, &lanec_suite_ev_lmi, &lanec_suite_di_transfer, &lanec_suite_di_transfer_c64, &lanec_suite_di_refuse, &lanec_suite_ev_transfer, &lanec_suite_ev_transfer_c64, &lanec_suite_ev_refuse, &lanec_suite_pw_blanksub, &lanec_suite_pw_decimal_time_consumer, &lanec_suite_pw_display_release, &lanec_suite_pw_enter, &lanec_suite_pw_enter_zero, &lanec_suite_pw_extended_dispatch, &lanec_suite_pw_five_blank, &lanec_suite_pw_fixed_display, &lanec_suite_pw_fraction_pair, &lanec_suite_pw_hours_output, &lanec_suite_pw_input_angle, &lanec_suite_pw_input_arithmetic, &lanec_suite_pw_input_factor, &lanec_suite_pw_input_mode, &lanec_suite_pw_input_scale, &lanec_suite_pw_input_select, &lanec_suite_pw_input_store, &lanec_suite_pw_input_time, &lanec_suite_pw_jam_control, &lanec_suite_pw_lamp_lifecycle, &lanec_suite_pw_lamp_release, &lanec_suite_pw_lamp_test, &lanec_suite_pw_load_complete, &lanec_suite_pw_load_control, &lanec_suite_pw_load_request, &lanec_suite_pw_load_store, &lanec_suite_pw_machine_cadr, &lanec_suite_pw_major_request, &lanec_suite_pw_minutes_output, &lanec_suite_pw_mixed_noun, &lanec_suite_pw_mode_admit };
const lanec_suite *lanec_find_suite(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i)
        if (!strcmp(suites[i]->name, name)) return suites[i];
    return 0;
}
