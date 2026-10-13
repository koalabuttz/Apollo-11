# SPDX-License-Identifier: MIT
# Milestone lanec-pw, batch g (lane C): the LM sides of CHARIN number and
# verb/noun, VERB/NOUN keys, DSPIN/11DSPIN/2BLANK, DPnOUTSF, DSPDECWD/NR,
# DSPDECVN and the Verb 30/31 operator scheduler (LM and CM) through the
# Lane C harness (tests/lanec_pw.h, tests/lanec_pw_g.h).
LANEC_SUITE_SRC += tests/lanec_pw.c tests/lanec_pw_charin_number_cases.c tests/lanec_pw_charin_nv_cases.c \
  tests/lanec_pw_nv_key_cases.c tests/lanec_pw_pinball_cases.c tests/lanec_pw_scale_cases.c \
  tests/lanec_pw_decimal_cases.c tests/lanec_pw_two_digit_cases.c tests/lanec_pw_operator_schedule_cases.c \
  src/agc_multiword.c src/dsky_pinball.c src/dsky_charin_number.c src/dsky_nv_key.c \
  src/dsky_charin_nv.c src/guidance_product.c src/dsky_scale.c src/dsky_decimal.c src/dsky_two_digit.c \
  src/executive.c src/lm_admission_image.c src/waitlist.c src/lm_waitlist_image.c \
  src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c \
  src/cm_admission_complete.c src/cm_waitlist_extended.c src/dsky_display_release.c src/dsky_request_control.c \
  src/dsky_operator_request.c src/dsky_operator_schedule.c \
  tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_number_cases.c tests/dsky_charin_nv_cases.c \
  tests/dsky_pinball_cases.c tests/dsky_scale_cases.c tests/dsky_decimal_cases.c tests/dsky_two_digit_cases.c \
  tests/dsky_operator_schedule_cases.c
LANEC_SUITE_HDR += tests/lanec_pw.h tests/lanec_pw_e.h tests/lanec_pw_g.h
LANEC_PW_SUITES += pw-charin-number pw-charin-nv pw-nv-key pw-pinball pw-scale pw-decimal pw-two-digit \
  pw-operator-schedule
