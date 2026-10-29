# SPDX-License-Identifier: MIT
# c64-dp-replay-link: the flight-replay harness (platform/c64/dap_pass_check.c,
# tools/flight_scenario_vice.py --replay) linked for each mission with the
# replay options scenarios turn on (tools/dp_replay_link.py: plain, busy,
# overload), nothing run. A resident that grows past the ABI 2 service table at $6F00
# fails here: in a PR that touches the resident, and on every push to the
# base, where the c64-maps job links every build/*.prg.
DP_REPLAY_LINK_PRGS = $(foreach m,lm cm,$(foreach p,plain busy overload,build/dp-replay-link-$(m)-$(p).prg))
.PHONY: c64-dp-replay-link ci-watch-c64-dp-replay-link
c64-dp-replay-link: $(DP_REPLAY_LINK_PRGS)
# The resident sources the harness links (tools/resident_flight_build.py
# resident_sources, both missions), named here: RF_SRC
# (tools/mk/resident-kernel.mk) is read after this file, so in a
# prerequisite list it was empty, and neither make nor tools/ci_affected.py
# saw a resident change (#941 then overflowed lm-overload on the base).
DP_REPLAY_LINK_SRC = platform/c64/resident_flight.c platform/c64/resident_flight.h platform/c64/resident_flight.ld \
  platform/c64/resident_cdu.S platform/c64/agc_find.S platform/c64/rk_clock.S platform/c64/overlay_v2_irq.S \
  platform/c64/overlay_reu_platform.c src/resident_rupt.c include/resident_rupt.h src/agc_image_ops.c \
  src/agc_multiword.c src/agc_transfer.c src/agc_word.c src/lm_exec_routines.c src/cm_exec_routines.c \
  src/lm_kernel.c src/cm_kernel.c src/lm_single_trig.c src/cm_quictrig.c src/cm_single_return.c \
  src/overlay_reu.c src/overlay_runtime.c src/overlay_v2.c src/sp_routines.c
$(DP_REPLAY_LINK_PRGS): build/dp-replay-link-%.prg: $(DP_REPLAY_LINK_SRC) platform/c64/dap_pass_check.c \
  include/resident_cdu.h tools/resident_flight_build.py tools/overlay_v2_services.py tools/dp_replay_link.py \
  tools/dap_pass_vice.py tools/flight_scenario_vice.py tests/dp_replay_link_snap.h | build
	python3 tools/dp_replay_link.py --cc $(MOS_CC) --mission $(word 1,$(subst -, ,$*)) --profile $(word 2,$(subst -, ,$*)) --out $@
# The resident's own files and the harness (the closure is every resident source).
ci-watch-c64-dp-replay-link: $(DP_REPLAY_LINK_SRC) platform/c64/dap_pass_check.c tools/resident_flight_build.py \
  tools/overlay_v2_services.py tools/dp_replay_link.py tools/dap_pass_vice.py tools/flight_scenario_vice.py \
  tests/dp_replay_link_snap.h
