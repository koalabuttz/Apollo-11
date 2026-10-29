# SPDX-License-Identifier: MIT
all: build/rtb-pulse-return-integrated-stream
build/rtb-pulse-return-integrated-stream: tools/rtb_pulse_return_snapshot.c src/rtb_pulse_return.c src/agc_word.c tests/rtb_pulse_return_cases.c tests/rtb_angles_cases.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-rtb-pulse-return-integrated
test-rtb-pulse-return-integrated:
	python3 tools/rtb_pulse_return_port_host.py --cc "$(CC)"
sanitize: sanitize-rtb-pulse-return-integrated
sanitize-rtb-pulse-return-integrated:
	python3 tools/rtb_pulse_return_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-rtb-pulse-return-integrated sanitize-rtb-pulse-return-integrated
