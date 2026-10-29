# SPDX-License-Identifier: MIT
SBAND_BODYFRAME_INTEGRATED_CORE = tests/sband_bodyframe_cases.c src/agc_word.c src/sband_bodyframe.c src/lm_powered_interface.c src/lm_powered_axis.c src/cm_powered_interface.c src/cm_powered_axis.c src/guidance_rotation.c src/cm_rotation.c src/guidance_product.c src/agc_multiword.c
SBAND_BODYFRAME_INTEGRATED_TEST = tests/test_sband_bodyframe.c tests/sband_bodyframe_target.c tests/frozen_words.c
build/sband-bodyframe-integrated-stream: tools/sband_bodyframe_snapshot.c $(SBAND_BODYFRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-bodyframe-integrated: $(SBAND_BODYFRAME_INTEGRATED_TEST) $(SBAND_BODYFRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-bodyframe: build/sband-bodyframe-integrated-stream build/test-sband-bodyframe-integrated
	./build/test-sband-bodyframe-integrated
	python3 tools/sband_bodyframe_port_host.py --binary build/sband-bodyframe-integrated-stream
sanitize-sband-bodyframe: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_BODYFRAME_INTEGRATED_TEST) $(SBAND_BODYFRAME_INTEGRATED_CORE) -o build/test-sband-bodyframe-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-bodyframe-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_bodyframe_snapshot.c $(SBAND_BODYFRAME_INTEGRATED_CORE) -o build/sband-bodyframe-integrated-stream-san
	python3 tools/sband_bodyframe_port_host.py --binary build/sband-bodyframe-integrated-stream-san
.PHONY: test-sband-bodyframe sanitize-sband-bodyframe
