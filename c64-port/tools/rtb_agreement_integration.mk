# SPDX-License-Identifier: MIT
RTB_AGREEMENT_INTEGRATED_CORE = src/rtb_agreement.c src/agc_word.c src/agc_multiword.c tests/rtb_agreement_cases.c
all: build/rtb-agreement-integrated-stream
build/rtb-agreement-integrated-stream: tools/rtb_agreement_snapshot.c $(RTB_AGREEMENT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-rtb-agreement-integrated
test-rtb-agreement-integrated: build/rtb-agreement-integrated-stream
	python3 tools/rtb_agreement_port_host.py --cc "$(CC)"
sanitize: sanitize-rtb-agreement-integrated
sanitize-rtb-agreement-integrated: | build
	python3 tools/rtb_agreement_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-rtb-agreement-integrated sanitize-rtb-agreement-integrated
