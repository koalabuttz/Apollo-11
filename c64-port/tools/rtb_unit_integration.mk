# SPDX-License-Identifier: MIT
RTB_UNIT_INTEGRATED_CORE = src/rtb_unit.c src/rtb_vector.c src/agc_word.c src/agc_multiword.c src/guidance_norm.c src/guidance_unit_phased.c src/guidance_length_phased.c tests/rtb_unit_cases.c tests/rtb_vector_cases.c
all: build/rtb-unit-integrated-stream
build/rtb-unit-integrated-stream: tools/rtb_unit_snapshot.c $(RTB_UNIT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-rtb-unit-integrated
test-rtb-unit-integrated: build/rtb-unit-integrated-stream
	python3 tools/rtb_unit_port_host.py --cc "$(CC)"
sanitize: sanitize-rtb-unit-integrated
sanitize-rtb-unit-integrated: | build
	python3 tools/rtb_unit_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-rtb-unit-integrated sanitize-rtb-unit-integrated
