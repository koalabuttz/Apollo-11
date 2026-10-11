# SPDX-License-Identifier: MIT
SBAND_LM_PITCH_INTEGRATED_CORE = tests/sband_lm_pitch_cases.c src/agc_word.c src/sband_lm_pitch.c src/sband_vector.c src/sband_shift.c src/sband_length.c src/guidance_length_phased.c src/interpreted_sign.c src/sband_dot.c src/sband_scalar.c src/inverse_trig.c src/guidance_shift.c src/guidance_product.c src/guidance_norm.c src/agc_multiword.c
SBAND_LM_PITCH_INTEGRATED_TEST = tests/test_sband_lm_pitch.c tests/sband_lm_pitch_target.c tests/frozen_words.c
build/sband-lm-pitch-integrated-stream: tools/sband_lm_pitch_snapshot.c $(SBAND_LM_PITCH_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-lm-pitch-integrated: $(SBAND_LM_PITCH_INTEGRATED_TEST) $(SBAND_LM_PITCH_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-lm-pitch: build/sband-lm-pitch-integrated-stream build/test-sband-lm-pitch-integrated
	./build/test-sband-lm-pitch-integrated
	python3 tools/sband_lm_pitch_port_host.py --binary build/sband-lm-pitch-integrated-stream
sanitize-sband-lm-pitch: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_LM_PITCH_INTEGRATED_TEST) $(SBAND_LM_PITCH_INTEGRATED_CORE) -o build/test-sband-lm-pitch-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-lm-pitch-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_lm_pitch_snapshot.c $(SBAND_LM_PITCH_INTEGRATED_CORE) -o build/sband-lm-pitch-integrated-stream-san
	python3 tools/sband_lm_pitch_port_host.py --binary build/sband-lm-pitch-integrated-stream-san
.PHONY: test-sband-lm-pitch sanitize-sband-lm-pitch
