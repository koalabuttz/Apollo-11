# SPDX-License-Identifier: MIT
# Milestone lanec-pw, batch f (lane C): the CM Pinball modules on a
# cm_native_io (CHARIN number and verb/noun, DSPIN/11DSPIN/2BLANK, the
# formatters, DSPDC2NR, M/SOUT, the V06N85, noun 40/85 and DYNDISP display
# routes) and LM NVSUBUSY through the Lane C harness (tests/lanec_pw.h).
LANEC_SUITE_SRC += tests/lanec_pw.c tests/lanec_pw_cm_pinball_cases.c tests/lanec_pw_cm_charin_number_cases.c \
  tests/lanec_pw_cm_charin_nv_cases.c tests/lanec_pw_cm_formatter_cases.c tests/lanec_pw_cm_decimal_pair_cases.c tests/lanec_pw_cm_scale_cases.c \
  tests/lanec_pw_cm_minutes_seconds_cases.c tests/lanec_pw_cm_nv85_cases.c tests/lanec_pw_cm_dynamic_display_cases.c \
  tests/lanec_pw_cm_normal_display_cases.c tests/lanec_pw_lm_pinball_busy_cases.c \
  src/agc_multiword.c src/dsky_pinball.c src/cm_pinball.c src/dsky_charin_number.c src/cm_charin_number.c \
  src/dsky_nv_key.c src/dsky_charin_nv.c src/cm_charin_nv.c src/dsky_two_digit.c src/dsky_decimal.c src/cm_formatter.c \
  src/cm_decimal_pair.c src/cm_minutes_seconds.c src/cm_interbank.c src/fixed_data.c src/guidance_product.c \
  src/cm_nv85.c src/cm_nv85_route.c src/cm_scale.c src/dsky_scale.c src/cm_clockjob.c src/cm_display_control.c \
  src/cm_makeplay.c src/cm_basic_priority.c src/cm_basic_wake.c src/cm_basic_finish.c src/cm_native_image.c \
  src/cm_executive.c src/cm_dispatch.c src/cm_normal_display.c src/cm_normal_display_route.c \
  src/cm_dynamic_display.c src/cm_dyndisp.c src/cm_dyndisp_route.c src/cm_powered_axis.c src/cm_powered_interface.c \
  src/cm_powered_trig.c src/cm_rotation.c src/gimbal_stage.c src/guidance_shift.c src/guidance_trig.c \
  src/matrix_native.c src/lm_pinball_busy.c \
  tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_number_cases.c tests/dsky_charin_nv_cases.c \
  tests/cm_pinball_cases.c tests/cm_charin_number_cases.c tests/cm_charin_nv_cases.c tests/cm_formatter_cases.c \
  tests/cm_decimal_pair_cases.c tests/cm_scale_cases.c tests/cm_minutes_seconds_cases.c tests/cm_nv85_cases.c tests/cm_clockjob_cases.c \
  tests/cm_trig_cases.c tests/cm_normal_display_cases.c tests/cm_dynamic_display_cases.c tests/cm_dyndisp_cases.c \
  tests/cm_dyndisp_apply.c tests/cm_nv85_apply.c tests/matrix_cases.c tests/throttle_bridge_cases.c tests/lm_busy_cases.c
LANEC_SUITE_HDR += tests/lanec_pw.h tests/lanec_pw_f.h
LANEC_PW_SUITES += pw-cm-pinball pw-cm-charin-number pw-cm-charin-nv pw-cm-formatter pw-cm-decimal-pair pw-cm-scale \
  pw-cm-minutes-seconds pw-cm-nv85 pw-cm-dynamic-display pw-cm-normal-display pw-lm-pinball-busy
