# SPDX-License-Identifier: MIT
# Unique targets only; the root build chooses which aggregate targets include them.
SBAND_SUPPORT_CONTROL_CORE = tests/sband_control_cases.c src/agc_word.c src/sband_control.c src/lm_interbank.c src/cm_interbank_complete.c src/service_routines.c
SBAND_SUPPORT_TRIG_CORE = tests/sband_trig_cases.c src/agc_word.c src/sband_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c
SBAND_SUPPORT_CONTROL_TEST = tests/test_sband_control.c tests/sband_control_target.c tests/frozen_words.c
SBAND_SUPPORT_TRIG_TEST = tests/test_sband_trig.c tests/sband_trig_target.c tests/frozen_words.c
build/sband-support-control-stream: tools/sband_control_snapshot.c $(SBAND_SUPPORT_CONTROL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/sband-support-trig-stream: tools/sband_trig_snapshot.c $(SBAND_SUPPORT_TRIG_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-support-control: $(SBAND_SUPPORT_CONTROL_TEST) $(SBAND_SUPPORT_CONTROL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-support-trig: $(SBAND_SUPPORT_TRIG_TEST) $(SBAND_SUPPORT_TRIG_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-support: build/sband-support-control-stream build/sband-support-trig-stream build/test-sband-support-control build/test-sband-support-trig
	./build/test-sband-support-control
	./build/test-sband-support-trig
	python3 tools/sband_support_port_host.py --control build/sband-support-control-stream --trig build/sband-support-trig-stream
sanitize-sband-support: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_SUPPORT_CONTROL_TEST) $(SBAND_SUPPORT_CONTROL_CORE) -o build/test-sband-support-control-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-support-control-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_SUPPORT_TRIG_TEST) $(SBAND_SUPPORT_TRIG_CORE) -o build/test-sband-support-trig-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-support-trig-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_control_snapshot.c $(SBAND_SUPPORT_CONTROL_CORE) -o build/sband-support-control-stream-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_trig_snapshot.c $(SBAND_SUPPORT_TRIG_CORE) -o build/sband-support-trig-stream-san
	python3 tools/sband_support_port_host.py --control build/sband-support-control-stream-san --trig build/sband-support-trig-stream-san
.PHONY: test-sband-support sanitize-sband-support
