# SPDX-License-Identifier: MIT
# Milestone lanec-pw (lane C), batch e: thirteen earlier Pinball modules
# through the Lane C harness (tests/lanec_pw.h, tests/lanec_pw_e.h). Hooked
# into test-lanec-pw / sanitize-lanec-pw by tools/mk/lanec-pw.mk.
LANEC_SUITE_SRC += tests/lanec_pw.c \
  tests/lanec_pw_octal_verb_cases.c tests/lanec_pw_operator_release_cases.c tests/lanec_pw_operator_request_cases.c \
  tests/lanec_pw_operator_response_cases.c tests/lanec_pw_read_pair_cases.c tests/lanec_pw_request_control_cases.c \
  tests/lanec_pw_response_recall_cases.c tests/lanec_pw_scalar_output_cases.c tests/lanec_pw_sign_output_cases.c \
  tests/lanec_pw_source_alarm_cases.c tests/lanec_pw_test_noun_cases.c tests/lanec_pw_two_integer_cases.c \
  tests/lanec_pw_verb_dispatch_cases.c \
  src/agc_multiword.c src/cm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_interbank_complete.c \
  src/cm_native_image.c src/dsky_display_release.c src/dsky_five_blank.c src/dsky_lamp_release.c src/dsky_octal_verb.c \
  src/dsky_operator_release.c src/dsky_operator_request.c src/dsky_operator_response.c src/dsky_pinball.c \
  src/dsky_read_pair.c src/dsky_request_control.c src/dsky_response_recall.c src/dsky_scalar_output.c \
  src/dsky_sign_output.c src/dsky_source_alarm.c src/dsky_test_noun.c src/dsky_two_integer.c src/dsky_verb_dispatch.c \
  src/executive.c src/fixed_data.c src/lm_basic_wake.c src/lm_interbank.c src/noun_reader.c \
  tests/dsky_octal_verb_cases.c tests/dsky_operator_release_cases.c tests/dsky_operator_request_cases.c \
  tests/dsky_operator_response_cases.c tests/dsky_read_pair_cases.c tests/dsky_request_control_cases.c \
  tests/dsky_response_recall_cases.c tests/dsky_scalar_output_cases.c tests/dsky_sign_output_cases.c \
  tests/dsky_source_alarm_cases.c tests/dsky_test_noun_cases.c tests/dsky_two_integer_cases.c \
  tests/dsky_verb_dispatch_cases.c tests/p66_bias_cases.c
LANEC_SUITE_HDR += tests/lanec_pw.h tests/lanec_pw_e.h
LANEC_PW_SUITES += pw-octal-verb pw-operator-release pw-operator-request pw-operator-response pw-read-pair \
  pw-request-control pw-response-recall pw-scalar-output pw-sign-output pw-source-alarm pw-test-noun \
  pw-two-integer pw-verb-dispatch
