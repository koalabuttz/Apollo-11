# SPDX-License-Identifier: MIT
SBAND_FRAME_INTEGRATED_CORE = tests/sband_frame_cases.c src/agc_word.c src/sband_frame.c src/rtb_unit.c src/rtb_vector.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/lm_powered_interface.c src/lm_powered_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c src/matrix_native.c src/sband_shift.c src/guidance_shift.c
SBAND_FRAME_INTEGRATED_TEST = tests/test_sband_frame.c tests/sband_frame_target.c tests/frozen_words.c
build/sband-frame-integrated-stream: tools/sband_frame_snapshot.c $(SBAND_FRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-frame-integrated: $(SBAND_FRAME_INTEGRATED_TEST) $(SBAND_FRAME_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-frame: build/sband-frame-integrated-stream
	for group in 0 1 2 3 4 5; do $(CC) $(CPPFLAGS) $(CFLAGS) -DSBAND_FRAME_GROUP=$$group $(SBAND_FRAME_INTEGRATED_TEST) $(SBAND_FRAME_INTEGRATED_CORE) -o build/test-sband-frame-integrated-$$group; ./build/test-sband-frame-integrated-$$group; done
	python3 tools/sband_frame_port_host.py --binary build/sband-frame-integrated-stream
sanitize-sband-frame: | build
	for group in 0 1 2 3 4 5; do $(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie -DSBAND_FRAME_GROUP=$$group $(SBAND_FRAME_INTEGRATED_TEST) $(SBAND_FRAME_INTEGRATED_CORE) -o build/test-sband-frame-integrated-san-$$group; ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-frame-integrated-san-$$group; done
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_frame_snapshot.c $(SBAND_FRAME_INTEGRATED_CORE) -o build/sband-frame-integrated-stream-san
	python3 tools/sband_frame_port_host.py --binary build/sband-frame-integrated-stream-san
.PHONY: test-sband-frame sanitize-sband-frame
