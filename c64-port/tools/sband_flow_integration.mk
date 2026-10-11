# SPDX-License-Identifier: MIT
SBAND_FLOW_INTEGRATED_CORE = tests/sband_flow_cases.c src/agc_word.c src/sband_flow.c src/lm_interbank.c src/cm_interbank_complete.c src/rtb_angles.c src/cm_powered_interface.c src/cm_powered_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c
SBAND_FLOW_INTEGRATED_TEST = tests/test_sband_flow.c tests/sband_flow_target.c tests/frozen_words.c
build/sband-flow-integrated-stream: tools/sband_flow_snapshot.c $(SBAND_FLOW_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-flow-integrated: $(SBAND_FLOW_INTEGRATED_TEST) $(SBAND_FLOW_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-flow: build/sband-flow-integrated-stream build/test-sband-flow-integrated
	./build/test-sband-flow-integrated
	python3 tools/sband_flow_port_host.py --binary build/sband-flow-integrated-stream
sanitize-sband-flow: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_FLOW_INTEGRATED_TEST) $(SBAND_FLOW_INTEGRATED_CORE) -o build/test-sband-flow-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-flow-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_flow_snapshot.c $(SBAND_FLOW_INTEGRATED_CORE) -o build/sband-flow-integrated-stream-san
	python3 tools/sband_flow_port_host.py --binary build/sband-flow-integrated-stream-san
.PHONY: test-sband-flow sanitize-sband-flow
