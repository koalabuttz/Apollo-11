# SPDX-License-Identifier: MIT
SBAND_NAVFRAME_INTEGRATED_CORE = tests/sband_navframe_cases.c src/agc_word.c src/sband_navframe.c src/sband_scalar.c src/rtb_unit.c src/rtb_vector.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/agc_multiword.c
SBAND_NAVFRAME_INTEGRATED_TEST = tests/test_sband_navframe.c tests/sband_navframe_target.c tests/frozen_words.c
build/sband-navframe-integrated-stream: tools/sband_navframe_snapshot.c $(SBAND_NAVFRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-navframe-integrated: $(SBAND_NAVFRAME_INTEGRATED_TEST) $(SBAND_NAVFRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-navframe: build/sband-navframe-integrated-stream build/test-sband-navframe-integrated
	./build/test-sband-navframe-integrated
	python3 tools/sband_navframe_port_host.py --binary build/sband-navframe-integrated-stream
sanitize-sband-navframe: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_NAVFRAME_INTEGRATED_TEST) $(SBAND_NAVFRAME_INTEGRATED_CORE) -o build/test-sband-navframe-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-navframe-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_navframe_snapshot.c $(SBAND_NAVFRAME_INTEGRATED_CORE) -o build/sband-navframe-integrated-stream-san
	python3 tools/sband_navframe_port_host.py --binary build/sband-navframe-integrated-stream-san
.PHONY: test-sband-navframe sanitize-sband-navframe
