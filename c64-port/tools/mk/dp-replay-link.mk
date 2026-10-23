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
$(DP_REPLAY_LINK_PRGS): build/dp-replay-link-%.prg: $(RF_SRC) platform/c64/dap_pass_check.c platform/c64/resident_cdu.S \
  include/resident_cdu.h tools/resident_flight_build.py tools/overlay_v2_services.py tools/dp_replay_link.py \
  tools/dap_pass_vice.py tools/flight_scenario_vice.py tests/dp_replay_link_snap.h | build
	python3 tools/dp_replay_link.py --cc $(MOS_CC) --mission $(word 1,$(subst -, ,$*)) --profile $(word 2,$(subst -, ,$*)) --out $@
# The resident's own files and the harness (the closure is every resident source).
ci-watch-c64-dp-replay-link: platform/c64/resident_flight.c platform/c64/resident_flight.h platform/c64/resident_cdu.S \
  src/resident_rupt.c include/resident_rupt.h platform/c64/dap_pass_check.c platform/c64/resident_flight.ld \
  tools/resident_flight_build.py tools/dp_replay_link.py tools/dap_pass_vice.py tools/flight_scenario_vice.py \
  tests/dp_replay_link_snap.h
