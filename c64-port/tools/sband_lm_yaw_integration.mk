# SPDX-License-Identifier: MIT
SBAND_LM_YAW_INTEGRATED_CORE = tests/sband_lm_yaw_cases.c src/agc_word.c src/sband_lm_yaw.c src/sband_trig.c src/guidance_trig.c src/sband_vector_linear.c src/sband_vector.c src/sband_shift.c src/sband_length.c src/guidance_length_phased.c src/interpreted_sign.c src/sband_dot.c src/sband_scalar.c src/inverse_trig.c src/guidance_shift.c src/guidance_product.c src/guidance_norm.c src/agc_multiword.c
SBAND_LM_YAW_INTEGRATED_TEST = tests/test_sband_lm_yaw.c tests/sband_lm_yaw_target.c tests/frozen_words.c
build/sband-lm-yaw-integrated-stream: tools/sband_lm_yaw_snapshot.c $(SBAND_LM_YAW_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-lm-yaw-integrated: $(SBAND_LM_YAW_INTEGRATED_TEST) $(SBAND_LM_YAW_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-lm-yaw: build/sband-lm-yaw-integrated-stream build/test-sband-lm-yaw-integrated
	./build/test-sband-lm-yaw-integrated
	python3 tools/sband_lm_yaw_port_host.py --binary build/sband-lm-yaw-integrated-stream
sanitize-sband-lm-yaw: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_LM_YAW_INTEGRATED_TEST) $(SBAND_LM_YAW_INTEGRATED_CORE) -o build/test-sband-lm-yaw-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-lm-yaw-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_lm_yaw_snapshot.c $(SBAND_LM_YAW_INTEGRATED_CORE) -o build/sband-lm-yaw-integrated-stream-san
	python3 tools/sband_lm_yaw_port_host.py --binary build/sband-lm-yaw-integrated-stream-san
.PHONY: test-sband-lm-yaw sanitize-sband-lm-yaw
