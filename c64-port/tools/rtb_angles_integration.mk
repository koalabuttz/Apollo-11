# SPDX-License-Identifier: MIT
RTB_ANGLES_INTEGRATED_CORE = src/rtb_angles.c src/agc_word.c src/agc_multiword.c tests/rtb_angles_cases.c
all: build/rtb-angles-integrated-stream
build/rtb-angles-integrated-stream: tools/rtb_angles_snapshot.c $(RTB_ANGLES_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-rtb-angles-integrated
test-rtb-angles-integrated: build/rtb-angles-integrated-stream
	python3 tools/rtb_angles_port_host.py --cc "$(CC)"
sanitize: sanitize-rtb-angles-integrated
sanitize-rtb-angles-integrated: | build
	python3 tools/rtb_angles_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-rtb-angles-integrated sanitize-rtb-angles-integrated
