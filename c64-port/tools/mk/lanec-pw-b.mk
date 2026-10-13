# SPDX-License-Identifier: MIT
# Milestone lanec-pw (lane C), batch b: fifteen earlier Pinball modules through
# the Lane C harness (tools/mk/lanec-pw.mk, tests/lanec_pw.h, tests/lanec_pw_b.h).
LANEC_SUITE_SRC += tests/lanec_pw.c \
  tests/lanec_pw_decimal_time_consumer_cases.c tests/lanec_pw_display_release_cases.c \
  tests/lanec_pw_enter_cases.c tests/lanec_pw_enter_zero_cases.c tests/lanec_pw_extended_dispatch_cases.c \
  tests/lanec_pw_five_blank_cases.c tests/lanec_pw_fixed_display_cases.c tests/lanec_pw_fraction_pair_cases.c \
  tests/lanec_pw_hours_output_cases.c tests/lanec_pw_input_angle_cases.c tests/lanec_pw_input_arithmetic_cases.c \
  tests/lanec_pw_input_factor_cases.c tests/lanec_pw_input_mode_cases.c tests/lanec_pw_input_scale_cases.c \
  tests/lanec_pw_input_select_cases.c \
  src/agc_multiword.c src/cm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_fixed_callers.c \
  src/cm_fixed_compat.c src/cm_native_image.c src/dsky_decimal_load.c src/dsky_decimal_raw.c \
  src/dsky_decimal_select.c src/dsky_decimal_time_consumer.c src/dsky_display_release.c src/dsky_enter.c \
  src/dsky_enter_zero.c src/dsky_extended_dispatch.c src/dsky_five_blank.c src/dsky_fixed_display.c \
  src/dsky_fixed_display_data.c src/dsky_fraction_pair.c src/dsky_hours_output.c src/dsky_input_angle.c \
  src/dsky_input_arithmetic.c src/dsky_input_factor.c src/dsky_input_mode.c src/dsky_input_scale.c \
  src/dsky_input_select.c src/dsky_minutes_output.c src/dsky_octal_display.c src/dsky_pinball.c \
  src/dsky_read_pair.c src/dsky_sign_output.c src/dsky_two_integer.c src/executive.c src/fixed_data.c \
  src/guidance_product.c src/lm_basic_wake.c src/lm_fixed_compat.c src/lm_interbank.c src/noun_reader.c \
  tests/dsky_decimal_time_consumer_cases.c tests/dsky_display_release_cases.c tests/dsky_enter_cases.c \
  tests/dsky_enter_zero_cases.c tests/dsky_extended_dispatch_cases.c tests/dsky_five_blank_cases.c \
  tests/dsky_fixed_display_cases.c tests/dsky_fraction_pair_cases.c tests/dsky_hours_output_cases.c \
  tests/dsky_input_angle_cases.c tests/dsky_input_arithmetic_cases.c tests/dsky_input_factor_cases.c \
  tests/dsky_input_mode_cases.c tests/dsky_input_scale_cases.c tests/dsky_input_select_cases.c \
  tests/p66_bias_cases.c
LANEC_SUITE_HDR += tests/lanec_pw.h tests/lanec_pw_b.h
LANEC_PW_SUITES += pw-decimal-time-consumer pw-display-release pw-enter pw-enter-zero pw-extended-dispatch \
  pw-five-blank pw-fixed-display pw-fraction-pair pw-hours-output pw-input-angle pw-input-arithmetic \
  pw-input-factor pw-input-mode pw-input-scale pw-input-select
