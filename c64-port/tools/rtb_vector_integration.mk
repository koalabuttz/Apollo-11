# SPDX-License-Identifier: MIT
RTB_VECTOR_INTEGRATED_CORE = src/rtb_vector.c src/agc_word.c src/agc_multiword.c tests/rtb_vector_cases.c
all: build/rtb-vector-integrated-stream
build/rtb-vector-integrated-stream: tools/rtb_vector_snapshot.c $(RTB_VECTOR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffunction-sections -Wl,--gc-sections $^ -o $@
test: test-rtb-vector-integrated
test-rtb-vector-integrated: build/rtb-vector-integrated-stream
	python3 tools/rtb_vector_port_host.py --cc "$(CC)"
sanitize: sanitize-rtb-vector-integrated
sanitize-rtb-vector-integrated: | build
	python3 tools/rtb_vector_port_host.py --cc "$(CC)" --sanitize
.PHONY: test-rtb-vector-integrated sanitize-rtb-vector-integrated
