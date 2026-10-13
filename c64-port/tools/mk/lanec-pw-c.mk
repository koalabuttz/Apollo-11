# SPDX-License-Identifier: MIT
# Milestone lanec-pw, batch c (lane C): fifteen earlier Pinball modules through
# the Lane C harness (tests/lanec_pw.h, tests/lanec_pw_c.h). Sources follow each
# module's own tools/dsky_<module>.mk CORE list; src/p66_native.c is taken as
# tests/lanec_p66_dispatch.c, its dispatch-only build.
LANEC_PW_C = input_store input_time jam_control lamp_lifecycle lamp_release lamp_test load_complete \
  load_control load_request load_store machine_cadr major_request minutes_output mixed_noun mode_admit
LANEC_SUITE_SRC += tests/lanec_pw.c $(foreach m,$(LANEC_PW_C),tests/lanec_pw_$(m)_cases.c src/dsky_$(m).c tests/dsky_$(m)_cases.c) \
  src/agc_multiword.c src/cm_admission_complete.c src/cm_basic_wake.c src/cm_dispatch.c src/cm_executive.c \
  src/cm_interbank_complete.c src/cm_native_image.c src/cm_waitlist_extended.c src/dsky_address_request.c \
  src/dsky_display_release.c src/dsky_five_blank.c src/dsky_fraction_pair.c src/dsky_input_mode.c \
  src/dsky_input_select.c src/dsky_noun_check.c src/dsky_nv_update_general.c src/dsky_octal_display.c \
  src/dsky_pinball.c src/dsky_response_recall.c src/dsky_source_alarm.c src/executive.c src/fixed_data.c \
  src/guidance_product.c src/lm_admission_image.c src/lm_basic_wake.c src/lm_interbank.c src/lm_waitlist_image.c \
  src/noun_reader.c src/waitlist.c tests/lanec_p66_dispatch.c tests/p66_bias_cases.c
LANEC_SUITE_HDR += tests/lanec_pw_c.h
LANEC_PW_SUITES += $(subst _,-,$(addprefix pw-,$(LANEC_PW_C)))
