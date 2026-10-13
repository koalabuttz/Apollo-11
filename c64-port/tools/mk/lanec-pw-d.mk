# SPDX-License-Identifier: MIT
# Milestone lanec-pw, batch d (lane C): mode job, monitor, noun check,
# NVSUB and octal display Pinball modules through the Lane C harness
# (tests/lanec_pw.h, tools/mk/lanec-pw.mk).
LANEC_SUITE_SRC += tests/lanec_pw_mode_job_cases.c tests/lanec_pw_monitor_consumer_cases.c \
  tests/lanec_pw_monitor_cases.c tests/lanec_pw_monitor_paste_cases.c tests/lanec_pw_noun_check_cases.c \
  tests/lanec_pw_nv_admission_cases.c tests/lanec_pw_nv_blank_cases.c tests/lanec_pw_nv_busy_cases.c \
  tests/lanec_pw_nv_consumer_cases.c tests/lanec_pw_nv_continuation_cases.c tests/lanec_pw_nv_return_cases.c \
  tests/lanec_pw_nv_update_general_cases.c tests/lanec_pw_nv_update_cases.c \
  tests/lanec_pw_octal_consumer_cases.c tests/lanec_pw_octal_display_cases.c \
  src/agc_multiword.c src/cm_formatter.c src/cm_interbank_complete.c src/cm_scale.c \
  src/dsky_blank.c src/dsky_blanksub.c src/dsky_decimal.c src/dsky_decimal_consumer.c src/dsky_decimal_general.c \
  src/dsky_decimal_load.c src/dsky_decimal_raw.c src/dsky_decimal_row.c src/dsky_decimal_select.c \
  src/dsky_machine_cadr.c src/dsky_mixed_noun.c src/dsky_mode_job.c src/dsky_monitor.c src/dsky_monitor_consumer.c \
  src/dsky_monitor_paste.c src/dsky_noun_check.c src/dsky_nv_admission.c src/dsky_nv_blank.c src/dsky_nv_busy.c \
  src/dsky_nv_consumer.c src/dsky_nv_continuation.c src/dsky_nv_return.c src/dsky_nv_update.c \
  src/dsky_nv_update_general.c src/dsky_octal_consumer.c src/dsky_octal_display.c src/dsky_octal_verb.c \
  src/dsky_pinball.c src/dsky_scalar_output.c src/dsky_scale.c src/dsky_test_noun.c src/dsky_two_digit.c \
  src/dsky_verb_dispatch.c src/fixed_data.c src/guidance_product.c src/lm_interbank.c src/noun_reader.c \
  tests/dsky_mode_job_cases.c tests/dsky_monitor_cases.c tests/dsky_monitor_consumer_cases.c \
  tests/dsky_monitor_paste_cases.c tests/dsky_noun_check_cases.c tests/dsky_nv_admission_cases.c \
  tests/dsky_nv_blank_cases.c tests/dsky_nv_busy_cases.c tests/dsky_nv_consumer_cases.c \
  tests/dsky_nv_continuation_cases.c tests/dsky_nv_return_cases.c tests/dsky_nv_update_cases.c \
  tests/dsky_nv_update_general_cases.c tests/dsky_octal_consumer_cases.c tests/dsky_octal_display_cases.c \
  tests/dsky_octal_verb_cases.c tests/p66_bias_cases.c
LANEC_SUITE_HDR += include/dsky_mode_job.h include/dsky_monitor_consumer.h include/dsky_monitor.h \
  include/dsky_monitor_paste.h include/dsky_noun_check.h include/dsky_nv_admission.h include/dsky_nv_blank.h \
  include/dsky_nv_busy.h include/dsky_nv_consumer.h include/dsky_nv_continuation.h include/dsky_nv_return.h \
  include/dsky_nv_update_general.h include/dsky_nv_update.h include/dsky_octal_consumer.h \
  include/dsky_octal_display.h tests/dsky_mode_job_cases.h tests/dsky_monitor_consumer_cases.h \
  tests/dsky_monitor_cases.h tests/dsky_monitor_paste_cases.h tests/dsky_noun_check_cases.h \
  tests/dsky_nv_admission_cases.h tests/dsky_nv_blank_cases.h tests/dsky_nv_busy_cases.h \
  tests/dsky_nv_consumer_cases.h tests/dsky_nv_continuation_cases.h tests/dsky_nv_return_cases.h \
  tests/dsky_nv_update_general_cases.h tests/dsky_nv_update_cases.h tests/dsky_octal_consumer_cases.h \
  tests/dsky_octal_display_cases.h
LANEC_PW_SUITES += pw-mode-job pw-monitor-consumer pw-monitor pw-monitor-paste pw-noun-check pw-nv-admission \
  pw-nv-blank pw-nv-busy pw-nv-consumer pw-nv-continuation pw-nv-return pw-nv-update-general pw-nv-update \
  pw-octal-consumer pw-octal-display
