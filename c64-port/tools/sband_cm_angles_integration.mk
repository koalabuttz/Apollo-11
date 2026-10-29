# SPDX-License-Identifier: MIT
SBAND_CM_ANGLES_INTEGRATED_CORE = tests/sband_cm_angles_cases.c src/agc_word.c src/sband_cm_angles.c src/sband_dot.c src/sband_scalar.c src/inverse_trig.c src/guidance_shift.c src/guidance_product.c src/guidance_norm.c src/agc_multiword.c
SBAND_CM_ANGLES_INTEGRATED_TEST = tests/test_sband_cm_angles.c tests/sband_cm_angles_target.c tests/frozen_words.c
build/sband-cm-angles-integrated-stream: tools/sband_cm_angles_snapshot.c $(SBAND_CM_ANGLES_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-cm-angles-integrated: $(SBAND_CM_ANGLES_INTEGRATED_TEST) $(SBAND_CM_ANGLES_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-cm-angles: build/sband-cm-angles-integrated-stream build/test-sband-cm-angles-integrated
	./build/test-sband-cm-angles-integrated
	python3 tools/sband_cm_angles_port_host.py --binary build/sband-cm-angles-integrated-stream
sanitize-sband-cm-angles: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_CM_ANGLES_INTEGRATED_TEST) $(SBAND_CM_ANGLES_INTEGRATED_CORE) -o build/test-sband-cm-angles-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-cm-angles-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_cm_angles_snapshot.c $(SBAND_CM_ANGLES_INTEGRATED_CORE) -o build/sband-cm-angles-integrated-stream-san
	python3 tools/sband_cm_angles_port_host.py --binary build/sband-cm-angles-integrated-stream-san
.PHONY: test-sband-cm-angles sanitize-sband-cm-angles
