# SPDX-License-Identifier: MIT
# Milestone lanec-pw (lane C), batch a: CHARIN-family and DECDSP Pinball
# modules through the Lane C harness (tools/mk/lanec-pw.mk runs the suites).
LANEC_SUITE_SRC += tests/lanec_pw_address_request_cases.c tests/lanec_pw_alarm_cycle_cases.c \
  tests/lanec_pw_alarm_recycle_cases.c tests/lanec_pw_blank_cases.c tests/lanec_pw_charin_alarm_cases.c \
  tests/lanec_pw_charin_clear_cases.c tests/lanec_pw_charin_release_cases.c tests/lanec_pw_charin_reset_cases.c \
  tests/lanec_pw_charin_sign_cases.c tests/lanec_pw_decimal_consumer_cases.c tests/lanec_pw_decimal_general_cases.c \
  tests/lanec_pw_decimal_load_cases.c tests/lanec_pw_decimal_raw_cases.c tests/lanec_pw_decimal_row_cases.c \
  tests/lanec_pw_decimal_select_cases.c \
  src/agc_multiword.c src/fixed_data.c src/noun_reader.c src/guidance_product.c \
  src/executive.c src/lm_basic_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c \
  src/dsky_pinball.c src/dsky_octal_display.c src/dsky_machine_cadr.c src/dsky_five_blank.c src/dsky_two_digit.c \
  src/cm_formatter.c src/dsky_decimal.c src/dsky_scale.c src/cm_scale.c src/dsky_scalar_output.c \
  src/dsky_nv_update_general.c src/dsky_address_request.c src/dsky_alarm_cycle.c src/dsky_alarm_recycle.c \
  src/dsky_blank.c src/dsky_charin_alarm.c src/dsky_charin_clear.c src/dsky_charin_release.c \
  src/dsky_charin_reset.c src/dsky_charin_sign.c src/dsky_decimal_load.c src/dsky_decimal_select.c \
  src/dsky_decimal_row.c src/dsky_decimal_general.c src/dsky_decimal_raw.c src/dsky_decimal_consumer.c \
  tests/p66_bias_cases.c tests/dsky_address_request_cases.c tests/dsky_alarm_cycle_cases.c \
  tests/dsky_alarm_recycle_cases.c tests/dsky_blank_cases.c tests/dsky_charin_alarm_cases.c \
  tests/dsky_charin_clear_cases.c tests/dsky_charin_release_cases.c tests/dsky_charin_reset_cases.c \
  tests/dsky_charin_sign_cases.c tests/dsky_decimal_consumer_cases.c tests/dsky_decimal_general_cases.c \
  tests/dsky_decimal_load_cases.c tests/dsky_decimal_raw_cases.c tests/dsky_decimal_row_cases.c \
  tests/dsky_decimal_select_cases.c
LANEC_SUITE_HDR += tests/lanec_pw_a.h
LANEC_PW_SUITES += pw-address-request pw-alarm-cycle pw-alarm-recycle pw-blank pw-charin-alarm pw-charin-clear \
  pw-charin-release pw-charin-reset pw-charin-sign pw-decimal-consumer pw-decimal-general pw-decimal-load \
  pw-decimal-raw pw-decimal-row pw-decimal-select
